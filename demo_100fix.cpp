#include <iostream>
#include <windows.h>
#include <Psapi.h>

#include "detours.h"

#include "plugin.h"

struct Signature
{
	Signature(const char* Bytes, size_t Length) { this->Bytes = Bytes; this->Length = Length; }
	const char* Bytes;
	size_t Length;
};

#define SIGNATURE_WILDCARD 0x2A
#define SIGNATURE(x) Signature( #x, sizeof(#x) - 1 )

uintptr_t FindSignature(uintptr_t Start, uintptr_t Size, const Signature& Sig)
{
	uintptr_t End = Start + Size - Sig.Length;
	for (uintptr_t i = Start; i < End; i++)
	{
		bool found = true;
		for (uintptr_t j = 0; j < Sig.Length; j++)
			found &= Sig.Bytes[j] == SIGNATURE_WILDCARD || Sig.Bytes[j] == *(char*)(i + j);
		if (found)
			return i;
	}
	return 0;
}

typedef void (*WarningFunc)(const char*, ...);
WarningFunc Warning = nullptr;

typedef uint32_t CBaseHandle;
typedef int ShouldTransmitState_t;
typedef int DataUpdateType_t;
class ClientClass;
class IClientUnknown;
class IClientEntity;
class bf_read;

class IClientNetworkable
{
public:
	virtual IClientUnknown*	GetIClientUnknown() = 0;
	virtual void			Release() = 0;
	virtual ClientClass*	GetClientClass() = 0;
	virtual void			NotifyShouldTransmit(ShouldTransmitState_t state) = 0;
	virtual void			OnPreDataChanged(DataUpdateType_t updateType) = 0;
	virtual void			OnDataChanged(DataUpdateType_t updateType) = 0;
	virtual void			PreDataUpdate(DataUpdateType_t updateType) = 0;
	virtual void			PostDataUpdate(DataUpdateType_t updateType) = 0;
	virtual bool			IsDormant(void) = 0;
	virtual int				entindex(void) const = 0;
	virtual void			ReceiveMessage(int classID, bf_read& msg) = 0;
	virtual void*			GetDataTableBasePtr() = 0;
	virtual void			SetDestroyedOnRecreateEntities(void) = 0;
	virtual void			OnDataUnchangedInPVS() = 0;
};

class IClientEntityList
{
public:
	virtual IClientNetworkable *	GetClientNetworkable(int entnum) = 0;
	virtual IClientNetworkable*		GetClientNetworkableFromHandle(CBaseHandle hEnt) = 0;
	virtual IClientUnknown*			GetClientUnknownFromHandle(CBaseHandle hEnt) = 0;
	virtual IClientEntity*			GetClientEntity(int entnum) = 0;
	virtual IClientEntity*			GetClientEntityFromHandle(CBaseHandle hEnt) = 0;
	virtual int						NumberOfEntities(bool bIncludeNonNetworkable) = 0;
	virtual void					SetMaxEntities(int maxents) = 0;
	virtual int						GetMaxEntities() = 0;
};

IClientEntityList* entitylist = nullptr;
#define VCLIENTENTITYLIST_INTERFACE_VERSION	"VClientEntityList003"

typedef void (*Func_CL_PreserveExistingEntity)(int);
Func_CL_PreserveExistingEntity CL_PreserveExistingEntity = nullptr;
void Hook_CL_PreserveExistingEntity(int nOldEntity)
{
	IClientNetworkable* pEnt = entitylist->GetClientNetworkable(nOldEntity);
	if (pEnt)
		pEnt->OnDataUnchangedInPVS();
	else
		Warning("CL_PreserveExistingEntity: missing client entity %d.\n", nOldEntity);
}

bool CPlugin::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	HMODULE tier0 = GetModuleHandle("tier0.dll");
	Warning = (WarningFunc)GetProcAddress(tier0, "Warning");
	if (!Warning)
	{
		DebugBreak();
		return false;
	}
	
	Warning(PLUGIN_NAME " Loading...\n");

	HMODULE client = GetModuleHandle("client.dll");
	CreateInterfaceFn clientFactory = (CreateInterfaceFn)GetProcAddress(client, "CreateInterface");
	if (!clientFactory)
	{
		DebugBreak();
		return false;
	}

	entitylist = (IClientEntityList*)clientFactory(VCLIENTENTITYLIST_INTERFACE_VERSION, NULL);
	if (!entitylist)
	{
		Warning(PLUGIN_NAME " Failed to retrieve entitylist interface\n");
		return false;
	}

	HMODULE engine = GetModuleHandle("engine.dll");

	MODULEINFO engineinfo = { 0 };
	GetModuleInformation(GetCurrentProcess(), engine, &engineinfo, sizeof(engineinfo));
	uintptr_t base = (uintptr_t)engineinfo.lpBaseOfDll;
	uintptr_t size = (uintptr_t)engineinfo.SizeOfImage;

#ifdef _WIN64
	CL_PreserveExistingEntity = (Func_CL_PreserveExistingEntity)FindSignature(base, size,
		SIGNATURE(\x40\x53\x48\x83\xEC\x20\x8B\xD9\x48\x8B\x0D\x2A\x2A\x2A\x2A\x8B));
#else
	CL_PreserveExistingEntity = (Func_CL_PreserveExistingEntity)FindSignature(base, size,
		SIGNATURE(\x55\x8B\xEC\x8B\x0D\x2A\x2A\x2A\x2A\xFF\x75\x08\x8B\x01\xFF\x10\x8B\xC8));
#endif
	if (!CL_PreserveExistingEntity)
	{
		Warning(PLUGIN_NAME " Failed to find CL_PreserveExistingEntity signature\n");
		return false;
	}

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&CL_PreserveExistingEntity, Hook_CL_PreserveExistingEntity);
	DetourTransactionCommit();

	Warning(PLUGIN_NAME " Success\n");
	return true;
}

void CPlugin::Unload()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&CL_PreserveExistingEntity, Hook_CL_PreserveExistingEntity);
	DetourTransactionCommit();
}
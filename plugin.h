#pragma once

#define PLUGIN_NAME "[Demo 100Fix]"

class CCommand;
struct edict_t;
typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);
typedef void* (*InstantiateInterfaceFn)();
typedef int QueryCvarCookie_t;

typedef enum
{
	PLUGIN_CONTINUE = 0,
	PLUGIN_OVERRIDE,
	PLUGIN_STOP
} PLUGIN_RESULT;

class CPlugin
{
public:
	virtual bool			Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
	virtual void			Unload(void);
	virtual void			Pause(void) {}
	virtual void			UnPause(void) {}
	virtual const char*		GetPluginDescription(void) { return PLUGIN_NAME; }
	virtual void			LevelInit(char const* pMapName) {}
	virtual void			ServerActivate(edict_t* pEdictList, int edictCount, int clientMax) {}
	virtual void			GameFrame(bool simulating) {}
	virtual void			LevelShutdown(void) {}
	virtual void			ClientActive(edict_t* pEntity) {}
	virtual void			ClientDisconnect(edict_t* pEntity) {}
	virtual void			ClientPutInServer(edict_t* pEntity, char const* playername) {}
	virtual void			SetCommandClient(int index) {}
	virtual void			ClientSettingsChanged(edict_t* pEdict) {}
	virtual PLUGIN_RESULT	ClientConnect(bool* bAllowConnect, edict_t* pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen) { return PLUGIN_CONTINUE; }
	virtual PLUGIN_RESULT	ClientCommand(edict_t* pEntity, const CCommand& args) { return PLUGIN_CONTINUE; }
	virtual PLUGIN_RESULT	NetworkIDValidated(const char* pszUserName, const char* pszNetworkID) { return PLUGIN_CONTINUE; }
	virtual void			OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t* pPlayerEntity, int eStatus, const char* pCvarName, const char* pCvarValue) {}
	virtual void			OnEdictAllocated(edict_t* edict) {}
	virtual void			OnEdictFreed(const edict_t* edict) {}
};

CPlugin g_Plugin;

extern "C" __declspec(dllexport) void* CreateInterface(const char* pName, int* pReturnCode)
{
	if (!strcmp("ISERVERPLUGINCALLBACKS003", pName))
	{
		if (pReturnCode)
			*pReturnCode = 0;
		return &g_Plugin;
	}

	if (pReturnCode)
		*pReturnCode = 1;
	return nullptr;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return TRUE;
}
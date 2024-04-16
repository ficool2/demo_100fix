# Demo 100Fixer plugin
Client-side plugin that fixes Host_Error: CL_PreserveExistingEntity when playing back demos from 100 player servers.

Compatible with 32-bit and 64-bit TF2.

## Installation
* Download the [latest release](https://github.com/ficool2/demo_100fix/releases) and extract the "addons" folder to TF2's main folder (../common/Team Fortress 2/tf).
* Run the game with -insecure in the launch parameters.
* If successful, you should see the plugin loaded in console.

To disable the plugin, remove -insecure from the launch parameters. The game will not load plugins if -insecure is not present in the launch parameters, and multiplayer servers can then be joined normally. To fully uninstall, simply remove the "addons" folder.
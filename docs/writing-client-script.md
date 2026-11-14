# Writing the Client Script
To write the client script, first create a file with the extension `.js` in a location together with the server DLL. 

The client script instantiates the server DLL using a standard JS function named `LoadStation` which is part of the Active Station Run-Time. Below is the definition of this function, which you must copy-paste to the beginning of your script code:

```js
var wshShell = new ActiveXObject("WScript.Shell");
var fso = new ActiveXObject("Scripting.FileSystemObject");

function LoadStation(moduleName) {
    var fileName = WSH.ScriptFullName;
    // var fileName = location.pathname;

    var cmd = "rundll32 " + moduleName + ",StationInject";
    var env = wshShell.Environment("Process");
    env("ActiveStation") = "?";
    var initialDir = wshShell.CurrentDirectory;
    wshShell.CurrentDirectory = fso.GetParentFolderName(fileName);
    if (!fso.FileExists(moduleName))
        throw new Error('The module "' + moduleName + '" could not be found');

    var exitCode = wshShell.Run(cmd, 0, true);
    wshShell.CurrentDirectory = initialDir;
    if (exitCode)
        throw new Error("StationInject has failed with error code " + exitCode);

    var station = GetObject(env("ActiveStation"));
    env.Remove("ActiveStation");

    return station;
}
```

The next step is to call the `LoadStation` function, passing it the relative path of your server DLL file. Then the `LoadStation` function will load the DLL and return the main COM object served by the DLL (i.e the Station object). Here is an example:

```js
var station = LoadStation("Test.dll");
```

> [!NOTE]
> Unfortunately, the `LoadStation` function does not accept paths that contain spaces. So if the name of your DLL file contains spaces, you must first rename it to remove the spaces, and then specify its new path in `LoadStation`.

Finally, you can use the methods and properties of the COM object that you have just obtained. For example:

```js
station.MoveCursor(600, 200);
```

Putting it all together, the full code of this client script would look like the following:

```js
var wshShell = new ActiveXObject("WScript.Shell");
var fso = new ActiveXObject("Scripting.FileSystemObject");

var station = LoadStation("Test.dll");
station.MoveCursor(600, 200);

function LoadStation(moduleName) {
    var fileName = WSH.ScriptFullName;
    // var fileName = location.pathname;

    var cmd = "rundll32 " + moduleName + ",StationInject";
    var env = wshShell.Environment("Process");
    env("ActiveStation") = "?";
    var initialDir = wshShell.CurrentDirectory;
    wshShell.CurrentDirectory = fso.GetParentFolderName(fileName);
    if (!fso.FileExists(moduleName))
        throw new Error('The module "' + moduleName + '" could not be found');

    var exitCode = wshShell.Run(cmd, 0, true);
    wshShell.CurrentDirectory = initialDir;
    if (exitCode)
        throw new Error("StationInject has failed with error code " + exitCode);

    var station = GetObject(env("ActiveStation"));
    env.Remove("ActiveStation");

    return station;
}
```

So far we have discussed how to write a client script that runs in [the Windows Script Host (WSH)](https://learn.microsoft.com/en-us/previous-versions/windows/internet-explorer/ie-developer/scripting-articles/9bbdkx3k(v=vs.84)) environment. It's worth noting that the client script can also be implemented as a script embedded in [an HTML Application (HTA)](https://learn.microsoft.com/en-us/previous-versions/ms536496(v=vs.85)). It's on your own to find out how.

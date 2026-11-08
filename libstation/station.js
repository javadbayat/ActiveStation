var wshShell = new ActiveXObject("WScript.Shell");
var fso = new ActiveXObject("Scripting.FileSystemObject");

function LoadStation(moduleName) {
    // var fileName = WSH.ScriptFullName;
    var fileName = location.pathname;

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
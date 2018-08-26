const fs = require("fs");
const os = require("os");
const cp = require('child_process');
const fileos = require("./fileos.js");
const ms = require("./mkshared.js");

var currpath = ms.currpath();

let oszip=module.exports;

oszip.unzip = function(zipfile,topath)
{
    if(os.platform() == "win32")
    {
        cp.execSync("call \"" +__dirname+ "\\..\\tool\\7za.exe\" x \""+ fileos.getPath(zipfile) + "\" -y -o\"" + fileos.getPath(topath)+"\"");
    }
    else if(os.platform() == "linux")
    {
        cp.execSync(fileos.getPath("unzip -o \"" + fileos.getPath(zipfile) + "\" -d \"" + fileos.getPath(topath)+"\""));
    }
}

oszip.zip=function(zipname,tmpdirname)
{
    if (os.platform() == "win32") {
        var cmd = "call \""+ __dirname + "\\..\\tool\\7za.exe\" a -tzip \""+ fileos.getPath(zipname)+ "\" \""+ fileos.getPath(tmpdirname)+"*\" -y";
        
        cp.execSync(cmd);
    }
    else if (os.platform() == "linux") {
		cp.execSync("cd "+fileos.getPath(tmpdirname)+";zip -q -r \""+fileos.getPath(zipname)+ "\" *;");
    }
}
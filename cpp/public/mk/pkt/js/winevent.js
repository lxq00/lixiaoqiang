const fs = require("fs");
const os = require("os");
const cp = require('child_process');

const fileos = require("./fileos.js");
const ms = require('./mkshared.js');
const log = require('./pktlog.js');

let winevent=module.exports;

winevent.event=function()
{
    var currpath = ms.currpath();
    var winevent="";
    if(os.platform() == "win32")
    {
        var wineventpath = fileos.getPath(currpath+"/__winevent.json");
        var evendata = cp.execSync("call \""+__dirname+"/../dos/getWin32Event.bat\" 2>nul").toString();
        while(evendata.indexOf("\\") > -1)
        {
            evendata = evendata.toString().replace("\\","{_point}");
        }
        while(evendata.indexOf("{_point}") > -1)
        {
            evendata = evendata.toString().replace("{_point}","\\\\");
        }
        while(evendata.indexOf("\"\"\"\"") > -1)
        {
            evendata = evendata.toString().replace("\"\"\"\"","\"\"");
        }
        winevent = JSON.parse(evendata);
    }
    return winevent;
}
const fs = require("fs");
const os = require("os");
const pr = require('process');
const cp = require('child_process');
const fileos = require("../js/fileos.js");

var eventobj={};

module.exports = class Event {
    event(){
        return eventobj;
    }
}

function _win32event()
{
    var currpath = pr.cwd();  
    var winevent="";
    if(os.platform() == "win32")
    {
        var evendata = cp.execSync("call \""+__dirname+"/../dos/win32Event_v3.bat\" 2>nul").toString();
        while(evendata.indexOf("\\") > -1)
        {
            evendata = evendata.toString().replace("\\","{_point}");
        }
        while(evendata.indexOf("{_point}") > -1)
        {
            evendata = evendata.toString().replace("{_point}","\\\\");
        }
        while(evendata.indexOf("\"\"") > -1)
        {
            evendata = evendata.toString().replace("\"\"","\"");
        }
        while(evendata.indexOf(":\",") > -1)
        {
            evendata = evendata.toString().replace(":\",",":\"\",");
        }
        winevent = JSON.parse(evendata);
    }
    return winevent;
}  

eventobj.win32=_win32event();
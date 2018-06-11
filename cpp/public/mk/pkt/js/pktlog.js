const fs = require("fs");
const os = require("os");
const cp = require('child_process');
const fileos = require("./fileos.js");
const ms = require('./mkshared.js');

let pktlog=module.exports;

pktlog.compilelog = function(prjcfg)
{
    return ms.logpath()+"/"+prjcfg.ProjectName;
}

pktlog.log = function(logstr)
{
    var logstr = fileos.date("yyyy/MM/dd hh:mm:ss :") + logstr;

    console.log(logstr);
    fs.appendFileSync(pktlog.logfile(),logstr+"\r\n");
}

pktlog.clean = function()
{
	fs.writeFileSync(pktlog.logfile(),"");
}

pktlog.logfile = function()
{
    return fileos.getPath(ms.logpath()+"//"+pktlog.getProjectName()+"_"+os.platform()+".log");
}

pktlog.getProjectName = function()
{
    var name = ms.currpath();
    while(1)
    {
        var pos = name.indexOf("\\");
        if(pos <= -1)   pos = name.indexOf("/");
        if(pos <= -1)   break;

        name = name.substring(pos + 1);
    }

    return name;
}

pktlog.printdebuginfo=function(lev,cmd,readme){

    function getstrlen(str){
        var outputstr = str;
        return outputstr.replace(/[\u0391-\uFFE5]/g,"aa").length;
    }

    var levsetup = 6;
    var cmdsetup = 60;

    var output = "";
    for(var i = 0;i < lev*levsetup;i++) output += " ";
    output += cmd;
    var printlen = getstrlen(output);
    for(var i = 0;i < cmdsetup - printlen;i ++)  output +=" ";
    output += readme;

    console.log(output);
}
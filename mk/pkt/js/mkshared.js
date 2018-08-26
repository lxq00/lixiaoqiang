const fs = require("fs");
const os = require("os");
const cp = require('child_process');
const fileos = require("./fileos.js");
const pr = require('process');

let mkshared=module.exports;

const sharedcfg = require("../sharedcfg.json");

///在这处理依赖文件下载路径
var sharedpath = sharedcfg.dependdir;

if(os.platform() == "linux")
{
    sharedpath = sharedcfg.linux.dependmountdir;
}

mkshared.dependpath = function()
{
    return sharedpath;
}


///在这处理日志文件路径

var logshredpath = sharedcfg.buildlog;
if(os.platform() == "linux")
{
    logshredpath = sharedcfg.linux.logmountdir;
}

mkshared.logpath = function()
{
    return logshredpath;
}

mkshared.currpath = function()
{
    return pr.cwd();
}

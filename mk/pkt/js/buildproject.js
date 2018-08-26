const fs = require("fs");
const os = require("os");
const cp = require('child_process');

const oscompile = require("./oscompile.js");

const log = require('./pktlog.js');
const fileos = require('./fileos.js');
const packet = require("./packetproject.js");

let buildproject = module.exports;


buildproject.build = function(prjpath,ver,debug)
{
    var cfg = JSON.parse(fs.readFileSync(fileos.getPath(prjpath + "/prjcfg.json")));

    log.log("Start Compile Project "+prjpath+"!");
    if (os.platform() == "win32") {
        oscompile.CompileWin32(cfg, prjpath,(cfg.Package && cfg.Package.Library && cfg.Package.Library.Valid.toLocaleLowerCase() == "true") || debug);
    }
    else if (os.platform() == "linux") {
        oscompile.CompileLinux(cfg, prjpath);
    }
    log.log("Success Compile Project，Start Package!");
    var pktlist = [];
    if (cfg.Package && cfg.Package.Library && cfg.Package.Library.Valid.toLocaleLowerCase() == "true")
    {
        packet.packLibrary(cfg, prjpath,ver);
    }
    else if (cfg.Package && cfg.Package.App && cfg.Package.App.Valid.toLocaleLowerCase() == "true") {
        pktlist = packet.packApp(cfg, prjpath,ver,debug);
    }

    return pktlist;
}

buildproject.buildpublic = function(prjpath,ver,buildcfg)
{
    var cfg = JSON.parse(fs.readFileSync(fileos.getPath(prjpath + "/prjcfg.json")));
    if(buildcfg.Compile)
    {
        log.log("Start Compile "+prjpath+"!");
        if (os.platform() == "win32") {
            oscompile.CompileWin32(cfg, prjpath,(cfg.Package && cfg.Package.Library && cfg.Package.Library.Valid.toLocaleLowerCase() == "true") || debug);
        }
        else if (os.platform() == "linux") {
            oscompile.CompileLinux(cfg, prjpath);
        }
    }

    packet.packLibrary(cfg, prjpath,ver,buildcfg);
}
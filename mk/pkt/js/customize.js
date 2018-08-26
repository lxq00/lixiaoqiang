const fs = require("fs");
const os = require("os");
const cp = require('child_process');

const oscompile = require("./oscompile.js");

const log = require('./pktlog.js');
const fileos = require('./fileos.js');
const packet = require("./packetproject.js");
const ms = require("./mkshared.js");

let customize = module.exports;


customize.build = function(ver,debug)
{
    var currpath = ms.currpath();

    var pktlist = [];

    var custiomizejson = fileos.getPath(currpath + "/customize.json.tpl");
    if (!fs.existsSync(custiomizejson))
    {
        return pktlist;
    }

    var cfg = JSON.parse(fs.readFileSync(custiomizejson));
    
    if(!cfg.customize || cfg.customize.length <= 0)
    {
        return pktlist;
    }

    for(var i = 0;i < cfg.customize.length;i ++)
    {
        if(!cfg.customize[i].name)
        {
            continue;
        }
        log.log("Start Package "+cfg.customize[i].name+"!");

        var debugzipname="";
        if(debug)
        {
            debugzipname = packet.pktCustomize(currpath,ver,cfg.customizeOutput,cfg.customize[i],debug,true);
        }        
        var zipname = packet.pktCustomize(currpath,ver,cfg.customizeOutput,cfg.customize[i],false,false);
        var fullzipname = packet.pktCustomize(currpath,ver,cfg.customizeOutput,cfg.customize[i],false,true);

        if(debugzipname != "")
        {
            pktlist.push(debugzipname);
        }
        if(zipname != "" && fullzipname != "")
        {
            pktlist.push(zipname);
            pktlist.push(fullzipname);
        }
    }

    return pktlist;
}
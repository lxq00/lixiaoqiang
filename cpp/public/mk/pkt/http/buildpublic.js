const os = require("os");
const fs = require("fs");
const cp = require('child_process');
const pr = require('process');
var route = require("./route.js");
var httpdata = require("./httpdata.js");
const fileos = require("../js/fileos.js");
const buildproject = require("../js/buildproject.js");
const filever = require("../js/updatever.js");
const printlog = require("../js/pktlog.js");

let buildpublic=module.exports;


var routetable = [
    {
        "method":"get",
         "url":"/buildpublic/connect",
        "do":connect
    },
    {
        "method":"post",
         "url":"/buildpublic/publish",
        "do":publish
    },
];

buildpublic.build=function(request,response)
{
    route.routePath(request,response,routetable);
}

function connect(request,response)
{
    var object= {};

    object["success"] = true;

    var outputval= new Array();
    var currscanpath = pr.cwd() + "/_Output/Include";
    if(fs.existsSync(fileos.getPath(currscanpath)))
    {
        files = fs.readdirSync(currscanpath);
        for(var i = 0;i < files.length;i ++)
        {
            var filepath = fileos.getPath(currscanpath +"/" +files[i]);
            var info = fs.statSync(filepath);
		    if(!info.isDirectory())
            {

            }

            var value = {};
            value["name"] = files[i];
            value["path"] = filepath;

            outputval.push(value);
        }
    }
    object["result"] = outputval;

    response.writeHeader(200);
    response.end(JSON.stringify(object));
}
function publish(request,response)
{
    printlog.clean();
    httpdata.parse(request,(data)=>{
        publishcfg = JSON.parse(data);
        var result = {};
        try {
            var ver = filever.scanAndUpdateVersion();
            buildproject.buildpublic(pr.cwd(),ver,publishcfg);
            result["success"] = true;
        } catch (error) {
            result["success"] = false;
            result["errormsg"] = error.message;
        }

        result["log"] = fs.readFileSync(printlog.logfile()).toString();
        response.writeHeader(200);
        response.end(JSON.stringify(result));
    });
}


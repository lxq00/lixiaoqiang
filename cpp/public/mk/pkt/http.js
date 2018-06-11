const currpath = process.cwd();
var http = require("http");
var url = require("url");
var buildpublic = require("./http/buildpublic.js");
var route = require("./http/route.js");

var routetable = [
    {
         "url":"/buildpublic",
        "do":buildpublic.build
    }
];


function parseRequest(request,response)
{
    route.routePath(request,response,routetable);
}

var httpsvr = http.createServer(parseRequest);
httpsvr.listen(3333);

console.log("server run listen 3333");

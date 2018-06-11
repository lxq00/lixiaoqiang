const os = require("os");
const fs = require("fs");
const cp = require('child_process');
var url = require("url");


let route=module.exports;

route.routePath = function(request,response,table)
{
    var requrl = url.parse(request.url);
    for(var i = 0;i < table.length;i ++)
    {
        var path = requrl.pathname;
        if(table[i].url.length < path.length)
        {
            path = path.substring(0,table[i].url.length);
        }
        if(path.toLocaleLowerCase() == table[i].url.toLocaleLowerCase())
        {
            return table[i].do(request,response);
        }
    }    

     response.writeHeader(404);
     response.end();
}

route.routeMethod = function(request,response,table)
{
    var requrl = url.parse(request.url);
    for(var i = 0;i < table.length;i ++)
    {
        if(requrl.method.toLocaleLowerCase() == table[i].method.toLocaleLowerCase()&&
        requrl.pathname.toLocaleLowerCase() == table[i].url.toLocaleLowerCase())
        {
            return table[i].do(request,response);
        }
    }    

     response.writeHeader(404);
     response.end();
}
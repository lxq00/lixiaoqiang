const os = require("os");
const fs = require("fs");
const cp = require('child_process');

let httpdata=module.exports;

httpdata.parse=function(httpbuffer,dofunc)
{
    var body = [];
    httpbuffer.on('data',function(chunk){
        body.push(chunk);

        dofunc(body);
    });
}

httpdata.parseToFile=function(httpbuffer,file)
{
    fs.writeFileSync(file,"");
    httpbuffer.on('data',function(chunk){
        fs.appendFileSync(file,chunk);
    });
}


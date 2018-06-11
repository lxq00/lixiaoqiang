const fs = require("fs");
const os = require("os");
const cp = require('child_process');
const path = require('path');
const log = require("../js/pktlog.js");

var cmdList = [];
class Helper{	
	doobj(){
		function publisfile(){
			if(os.platform() == "win32") return "Publish.bat";
			else return "Publish.sh";
		}
		console.log("---------------------构建打包工具介绍 V3.0 ---------------------------");
		console.log("使用方法："+publisfile()+" [cmd] [option]");
		console.log("");
		for(var i = 0;i < cmdList.length;i ++){
            log.printdebuginfo(1,cmdList[i].cmd,cmdList[i].readme);
			if(cmdList[i].object && cmdList[i].object.about) {
				cmdList[i].object.about();
			}
            console.log("");
		}
		console.log("----------------------------------------------------------------------");
	}
}

let main=module.exports;
main.initHelper=function(cmd,readme)
{
    main.registCmdHandle(cmd,new Helper(),readme);
}
main.registCmdHandle = function(cmd,object,readme){
    var cmdobj = {};
    cmdobj.cmd = cmd;
    cmdobj.readme = readme;
    cmdobj.object = object;

    cmdList = cmdList.concat(cmdobj);
}
main.doCmdHandle = function(cmd,args){
    function getCmdobj(cmd){
        for(var i = 0;i < cmdList.length;i ++){
            var cmdarray = new Array();
            cmdarray = cmdList[i].cmd.split("/");
            for(var j = 0;j < cmdarray.length;j ++){
                if(cmdarray[j].toLocaleLowerCase()  == cmd.toLocaleLowerCase()){
                    return cmdList[i];
                }
            }
        }
        return null;
    }
    var cmdobj = getCmdobj(cmd);
    if(!cmdobj){
        log.log("暂不支持的命令["+cmd+"]");
        return;
    }
    cmdobj.object.doobj(args);
}
const currpath = process.cwd();
const os = require("os");
const fs = require("fs");
const cp = require('child_process');

const log = require("./js/pktlog.js");
const build = require("./js/buildproject.js");
const filever = require("./js/updatever.js");
const download = require("./js/downloaddepend.js");
const fileos = require("./js/fileos.js");
const oszip = require("./js/oszip.js");
const customize = require("./js/customize.js");

log.clean();

if(!fs.existsSync(fileos.getPath(currpath+"/pkgcfg.json")))
{
	update.getDCPkgcfg(currpath,"pkgcfg.json");
}

function runProject(ver,debug)
{
	log.log("开始对项目进行编译并打包...");

	pkgcfg = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/pkgcfg.json")));

    if(pkgcfg.Project.length == 0)
    {
        pkgcfg.Project.push("");
    }
	var pktlist = [];

	if ((os.platform() == "win32" && pkgcfg.Project && (pkgcfg.Project.win32 || pkgcfg.Project.common)) || 
		(os.platform() == "linux" && pkgcfg.Project && (pkgcfg.Project.linux || pkgcfg.Project.common)))
	{
		var project = (os.platform() == "win32") ? pkgcfg.Project.win32 : pkgcfg.Project.linux;
		project = project ? project.concat(pkgcfg.Project.common) : project;

		for (var i = 0; project && i < project.length; i++) {
			if(os.platform() == "linux" && project[i] != "")
			{
				fileos.copyDir(currpath+"/mk",currpath+"/"+project[i]);
			}
			log.log("开始对"+(project[i] == "" ? "当前":project[i])+"项目进行编译打包...");
			var pkt = build.build(fileos.getPath(currpath+"/"+project[i]),ver,debug);
			if(os.platform() == "linux" && project[i] != "")
			{
				fileos.rmdir(currpath+"/"+project[i]+"/mk");
			}
			pktlist = pktlist.concat(pkt);
		}
	}
	else
	{
		for (var i = 0; i < pkgcfg.Project.length; i++) {
			log.log("开始对"+(pkgcfg.Project[i] == "" ? "当前":pkgcfg.Project[i])+"项目进行编译打包...");
			var pkt = build.build(fileos.getPath(currpath+"/"+pkgcfg.Project[i]),ver,debug);
			pktlist = pktlist.concat(pkt);
		}
	}
	{
		var pkt = customize.build(ver,debug);
		pktlist = pktlist.concat(pkt);
	}


	if(pktlist.length > 0)
	{
		var pktmk = fileos.getPath(currpath+"\\"+log.getProjectName()+"_output");
		if (fs.existsSync(pktmk))
		{
			fileos.rmdir(pktmk);
		}
		fs.mkdirSync(pktmk);
		for(var i = 0;i < pktlist.length;i ++)
		{
			fileos.copy(pktlist[i],pktmk);
			fileos.rmfile(pktlist[i]);
		}
		fileos.rmfile(currpath+"\\"+log.getProjectName()+".zip");
		oszip.zip(currpath+"\\"+log.getProjectName()+".zip",pktmk+"\\");
		fileos.rmdir(pktmk);
	}

	log.log("项目打包成功!");
}

function updateVersion()
{
	log.log("开始检测更新版本号...");
	pktversion = filever.updateVersion(currpath,winevent);
	log.log("更新版本号结束！");
}

function runBuild(debug)
{
    download.download();

	var ver = filever.scanAndUpdateVersion();
	
    runProject(ver,debug);    
}




var arg = process.argv.splice(2);


if(arg.length > 0 && (arg[0].toLocaleLowerCase() == "help" || arg[0].toLocaleLowerCase() == "?"))
{
	console.error("-----------------------构建打包工具介绍-----------------------------");
	console.error("使用方法：");
	console.error("Publish.bat help/?		显示当前帮助文档");
	console.error("Publish.bat version		更新仓库版本号信息，请先手动修改Version.tpl文件");
	console.error("Publish.bat depend		手动下载依赖文件，用于手动编译使用");
	console.error("Publish.bat debug		手动打包Debug版本发布包");
	console.error("Publish.bat publish		更新项目依赖包的版本号");
	console.error("Publish.bat clean		清除项目依赖包的版本号");
	console.error("Publish.bat			手动打包Realse版本发布包");
	console.error("");
	console.error("注：该打包工具在Win10系统下必须使用管理员身份执行!");
	console.error("");

	var localversion;
    if(fs.existsSync(fileos.getPath(currpath + "/buildtoolver.json")))
    {
        localversion = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/buildtoolver.json"),"utf-8"));
    }

	console.error("当前构建工具版本号为："+(localversion ? localversion.version:"0.0.0.0"));
	console.error("----------------------------------------------------------------------");
	return;
}

var isdebug = false;

if(arg.length > 0 && arg[0].toLocaleLowerCase() == "debug")
{
	isdebug = true;
}

try {
	if(arg.length > 0 && arg[0].toLocaleLowerCase() == "version")
	{
		filever.scanAndUpdateVersion();
	}
	else if(arg.length > 0 && arg[0].toLocaleLowerCase() == "depend")
	{
		download.download();
	}
	else if(arg.length > 0 && arg[0].toLocaleLowerCase() == "publish")
	{
		download.publish();
	}
	else if(arg.length > 0 && arg[0].toLocaleLowerCase() == "clean")
	{
		download.clean();
	}
	else
	{
		runBuild(isdebug);
	}	
} catch (error) {
	log.log(error);
}

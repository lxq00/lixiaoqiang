const fs = require("fs");
const os = require("os");
const cp = require('child_process');

const ms = require('./mkshared.js');
const log = require('./pktlog.js');
const fileos = require('./fileos.js');
const event = require("./winevent.js");
const verobj = require("./version.js");


let updatever=module.exports;

function getGitVersion(currpath,winevent)
{
	var gitverdata = "";
	if (os.platform() == "win32") {            
		var gitpath = "";
		if(winevent && winevent.git)
		{
			gitpath = winevent.git;
		}
		if(gitpath == "")
		{
			console.error("Get Git Version Error,Not Found Git！");
			return 0;
		}
		var cmd  = "call \""+gitpath+"/bin/sh.exe\" \""+currpath+"\\mk\\pkt\\sh\\gitversion.sh\"";
		gitverdata = cp.execSync(fileos.getPath(cmd));
	}
	else if (os.platform() == "linux") {
		var cmd  = "\""+__dirname+"/../sh/gitversion.sh\"";
		gitverdata =  cp.execSync(fileos.getPath(cmd));
	}
	var version = JSON.parse(gitverdata.toString());
	return version.gitver;
}


updatever.scanAndUpdateVersion = function()
{
    var currpath = ms.currpath();
    var winevent = event.event();

    log.log("Start Scan Version File！");

	if(!fs.existsSync(fileos.getPath(currpath + "/Version.tpl")))
	{
		return "";
	}
	
	var version = JSON.parse(fileos.readFileSync(fileos.getPath(currpath + "/Version.tpl")));
	var ver = verobj.buildFullVersion(version.productVersion);
	if(ver == "")
	{
		log.log("Build Version Error,Parse Version.tpl Error,Path:"+currpath);
		return "";
	}

	if(fs.existsSync(fileos.getPath(currpath + "/prjcfg.json")))
	{
		var cfg = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/prjcfg.json")));
		if(cfg && cfg.Package && cfg.Package.Library && cfg.Package.Library.Valid && cfg.Package.Library.Valid.toLocaleLowerCase() == "true" && ver[2] == 0 && cfg.ProjectName != "vgsii_decoder")
		{
			ver[2] = getGitVersion(currpath,winevent);
		}
	}
	
	var pktversion = ver[0]+"."+ver[1]+"."+ver[2];
	var rcvertmp = fileos.readFileSync(fileos.getPath(__dirname + "/../tpl/versionrc.tpl"));
    log.log("Start Scan Version.tpl And Update Version！");
	scanAndUpdateVersion(ver,currpath,rcvertmp,version.productVersionAlias ? version.productVersionAlias:"",currpath);
    log.log("Success Scan Version.tpl And Update Version");
    
	return pktversion;
}
function scanAndUpdateVersion(version,path,rcvertmp,VersionAlias,currpath)
{
	files = fs.readdirSync(path);
	var dirarray = new Array();
	for(var i = 0;i < files.length;i ++)
	{
		if(files[i].substring(0,1) == "." || files[i] == "_Output" || files[i] == "__Compile" || files[i] == "Include" || files[i] == "Lib")
		{
			continue;
		}
		var info = fs.statSync(fileos.getPath(path +"/" +files[i]));
		if(info.isDirectory())
		{
			dirarray.push(files[i]);
			continue;
		}
		else if(files[i] != "WinVersion.tpl")
		{
			continue;
		}
		var versiontmp = JSON.parse(fileos.readFileSync(fileos.getPath(path + "/WinVersion.tpl")));
		if(versiontmp && versiontmp.filename && versiontmp.productname)
		{
			writeVersionini(version,versiontmp,path,VersionAlias);
			writeWinVersion(version,versiontmp,path,rcvertmp,VersionAlias,currpath);
		}
	}
	for(var i = 0;i < dirarray.length;i ++)
	{
		scanAndUpdateVersion(version,fileos.getPath(path +"/" +dirarray[i]),rcvertmp,VersionAlias,currpath);
	}
}


function writeVersionini(vers,versiontmp,path,VersionAlias)
{
	var verdata  = "#ifndef ___VERSION__H__\r\n";
		verdata += "#define ___VERSION__H__\r\n";
		verdata += "static const char versionalias[] = \"{versionalias}\";\r\n";
		verdata += "static const int r_major = {major};\r\n";
		verdata += "static const int r_minor = {minor};\r\n";
		verdata += "static const int r_build = {build};\r\n";
		verdata += "#endif  //___VERSION__H__\r\n";

	verdata = verdata.replace("{major}",vers[0]);
	verdata = verdata.replace("{minor}",vers[1]);
	verdata = verdata.replace("{build}",vers[2]);
	verdata = verdata.replace("{versionalias}",VersionAlias);

	fs.writeFileSync(fileos.getPath(path + "/version.inl"), verdata);
}

function writeWinVersion(vers,versiontmp,path,rcvertmp,VersionAlias,currpath)
{
	var verdata = rcvertmp;
    
    while(verdata.indexOf("{major}") > -1)
    {
        verdata = verdata.replace("{major}",vers[0]);
    }
    while(verdata.indexOf("{minor}") > -1)
    {
        verdata = verdata.replace("{minor}",vers[1]);
    }
    while(verdata.indexOf("{build}") > -1)
    {
        verdata = verdata.replace("{build}",vers[2]);
    }
    while(verdata.indexOf("{filename}") > -1)
    {
        verdata = verdata.replace("{filename}",versiontmp.filename);
    }
	verdata = verdata.replace("{VersionAlias}",VersionAlias ? VersionAlias : versiontmp.filename);

	verdata = verdata.replace("{productname}",versiontmp.productname);
	verdata = verdata.replace("{year}",fileos.date("yyyy"));

	fileos.writeFileSync(path + "/Version.rc", verdata);
}

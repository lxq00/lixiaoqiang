const fs = require("fs");
const os = require("os");
const cp = require('child_process');
const fileos = require("./fileos.js");
const fwinevent = require("./winevent.js");
const log = require("./pktlog.js");

let oscompile=module.exports;


function win32Run(cfg,sln, prjpath,type,debug)
{
    winevent = fwinevent.event();
    var bin = winevent.msbuild;

    var msbuilder = true;
    if(type == 2008 && bin == "" && winevent.vs2008 != "")
    {
        bin = winevent.vs2008;
        msbuilder = false;
    }
    else if(type == 2012 && bin == "" && winevent.vs2012 != "")
    {
        bin = winevent.vs2012;
        msbuilder = false;
    }
    else if(type == 2012 && bin == "" && winevent.vs2015 != "")
    {
        bin = winevent.vs2015;
        msbuilder = false;
    }

	cp.execSync("cd "+prjpath);

	if(fs.existsSync(bin) && fs.existsSync(sln))
    {
        var param = "";
        var outputfile = fileos.getPath(log.compilelog(cfg) +"_"+ type+"_"+(debug?"Debug":"Release")+"_output.txt");
        if(msbuilder)
        {
            param = "/t:Rebuild /p:Configuration="+(debug?"Debug":"Release") + " > \"" + outputfile+"\"";
        }
        else
        {
            param = "/rebuild "+(debug?"Debug":"Release")+" /out \"" + outputfile+"\"";
        }
        var cmd = "cd "+prjpath + " && call \"" +bin + "\" \""+ sln+ "\" "+ param;
        log.log(cmd);
        cp.execSync(cmd);
    }
    else
    {
        log.log("Not Find  VSBin:"+bin+" Sln:"+sln + " describe :" + type );
		process.exit();
    }
}
oscompile.CompileWin32=function(cfg, prjpath,debug) {
	if (fs.existsSync(fileos.getPath(prjpath + "/beforeCompile.bat")))
    {
        cp.execSync(fileos.getPath(prjpath + "/beforeCompile.bat "+prjpath));
    }
    if (cfg.Compile && cfg.Compile.Win32 && cfg.Compile.Win32.vs2008 && debug) {
        win32Run(cfg,prjpath + "\\" + cfg.Compile.Win32.vs2008,prjpath,2008,true);
    }

    if (cfg.Compile && cfg.Compile.Win32 && cfg.Compile.Win32.vs2008) {
        win32Run(cfg,prjpath + "\\" + cfg.Compile.Win32.vs2008,prjpath,2008,false);
    }

    if (cfg.Compile && cfg.Compile.Win32 && cfg.Compile.Win32.vs2012 && debug) {
        win32Run(cfg,prjpath + "\\" + cfg.Compile.Win32.vs2012,prjpath,2012,true);
    }

    if (cfg.Compile && cfg.Compile.Win32 && cfg.Compile.Win32.vs2012) {
        win32Run(cfg,prjpath + "\\" + cfg.Compile.Win32.vs2012,prjpath,2012,false);
    }

     if (cfg.Compile && cfg.Compile.Win32 && cfg.Compile.Win32.vs2015 && debug) {
        win32Run(cfg,prjpath + "\\" + cfg.Compile.Win32.vs2015,prjpath,2015,true);
    }

    if (cfg.Compile && cfg.Compile.Win32 && cfg.Compile.Win32.vs2015) {
        win32Run(cfg,prjpath + "\\" + cfg.Compile.Win32.vs2015,prjpath,2015,false);
    }
}

oscompile.CompileLinux=function(cfg, prjpath) {
    if (fs.existsSync(fileos.getPath(prjpath + "/beforeCompile.sh"))) {
        cp.execSync("cd "+prjpath+"&&/bin/sh "+fileos.getPath(prjpath + "/beforeCompile.sh "+ prjpath));
    }
    var compliefile = fileos.getPath(log.compilelog(cfg) + "_linux.Compile.txt");
    var cmdstr = "cd "+prjpath+"&&make clean&&make > "+compliefile;
    log.log(cmdstr);
    cp.execSync(cmdstr);
}

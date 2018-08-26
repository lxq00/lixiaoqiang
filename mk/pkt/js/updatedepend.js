const fs = require("fs");
const os = require("os");
const cp = require('child_process');

const ms = require('./mkshared.js');
const log = require('./pktlog.js');
const fileos = require('./fileos.js');
const oszip = require("./oszip.js");

let updatedepend=module.exports;

updatedepend.updateOutput = function(version,prjcfg,buildcfg)
{
    files = fs.readdirSync(fileos.getPath(ms.currpath() + "/_Output/Include"));
	log.log("Start Scan Update Output DIR!");
    for(var i = 0;i < files.length;i ++)
    {
        if(buildcfg)
        {
            var find = false;
            if(buildcfg.Result)
            {
                for(var j = 0;j < buildcfg.Result.length;j ++)
                {
                    if(files[i].toLocaleLowerCase() == buildcfg.Result[j].toLocaleLowerCase())
                    {
                        find = true;
                        break;
                    }
                }
                if(!find)
                    continue;
            }
        }
        else if(isInExclude(files[i],prjcfg))
        {
            continue;
        }
        log.log("Start Update "+files[i]);
        zipUpdateDirAndUpdate(ms.currpath(),version,files[i]);
    }
    log.log("Success Scan Update Output DIR!");
}

updatedepend.updateBin = function(prjpath,version,prjname)
{
    if(!fs.existsSync(fileos.getPath(prjpath+"/Bin/"+(os.platform() == "win32"?"win32":"x86"))))
    {
        return;
    }
    log.log("Start Scan Update Bin DIR "+prjname);
    var currpath = ms.currpath();
    var mkdirdir = prjpath + "/__" + prjname + "__tmp";
    if(fs.existsSync(fileos.getPath(mkdirdir)))
    {
        rmdirforce(fileos.getPath(mkdirdir));
    }
    fs.mkdirSync(fileos.getPath(mkdirdir));
    fs.mkdirSync(fileos.getPath(mkdirdir+"/Bin"));
    fs.mkdirSync(fileos.getPath(mkdirdir+"/Bin/"+(os.platform() == "win32"?"win32":"x86")));
    fileos.copyDir(prjpath+"/Bin/"+(os.platform() == "win32"?"win32":"x86")+"/*",mkdirdir+"/Bin/"+(os.platform() == "win32"?"win32":"x86"));

    var zipname = mkdirdir+"/"+prjname+"_"+os.platform()+"_"+version+".zip";
    oszip.zip(zipname,mkdirdir+"/");

    updateTo(prjname,zipname);
    rmdirforce(fileos.getPath(mkdirdir));

    log.log("Success Scan Update Bin DIR "+prjname);
}

updatedepend.updateLib = function(prjpath,version,prjcfg)
{
    var readpath = prjpath + "/Lib/"+(os.platform() == "win32"?"win32":"x86");

    files = fs.readdirSync(fileos.getPath(readpath));
	log.log("Start Scan Update Lib DIR!");
    for(var i = 0;i < files.length;i ++)
    {
        if(isInExclude(files[i],prjcfg))
        {
            continue;
        }
        log.log("Start Update "+files[i]);
        zipUpdateDirAndUpdateLib(prjpath,version,files[i]);
    }
    log.log("Success Scan Update Lib DIR!");
}

function isInExclude(name,prjcfg)
{
    if(prjcfg && prjcfg.Package && prjcfg.Package.Library && prjcfg.Package.Library.Valid && prjcfg.Package.Library.Exclude)
    {
        for(var i = 0;i < prjcfg.Package.Library.Exclude.length;i ++)
        {
            if(name.toLocaleLowerCase() == prjcfg.Package.Library.Exclude[i].toLocaleLowerCase())
            {
                return true;
            }
        }
    }

    return false;
}
function zipUpdateDirAndUpdate(currpath,version,name)
{
    var mkdirdir = currpath + "/__" + name + "__tmp";
    if(fs.existsSync(fileos.getPath(mkdirdir)))
    {
        rmdirforce(fileos.getPath(mkdirdir));
    }
    fs.mkdirSync(fileos.getPath(mkdirdir));
    fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output"));
    fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output/Include"));
    fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output/Include/"+name));

    fileos.copyDir(currpath+"/_Output/Include/"+name+"/*",mkdirdir+"/_Output/Include/"+name+"/");

    if(fs.existsSync(fileos.getPath(currpath + "/_Output/Lib/"+(os.platform() == "win32"?"win32":"x86")+"/"+ name)))
    {
        fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output/Lib"));
        fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output/Lib/"+(os.platform() == "win32"?"win32":"x86")));
        fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output/Lib/"+(os.platform() == "win32"?"win32":"x86")+"/"+ name));
        fileos.copyDir(currpath + "/_Output/Lib/"+(os.platform() == "win32"?"win32":"x86")+"/"+ name+"/*",mkdirdir+"/_Output/Lib/"+(os.platform() == "win32"?"win32":"x86")+"/"+ name+"/");
    }

    var zipname = mkdirdir+"//"+name+"_"+os.platform()+"_"+version+".zip";
    oszip.zip(zipname,mkdirdir+"/");
    
    updateTo(name,zipname);
    rmdirforce(fileos.getPath(mkdirdir));
}
function zipUpdateDirAndUpdateLib(prjpath,version,name)
{
    var mkdirdir = prjpath + "/__" + name + "__tmp";
    if(fs.existsSync(fileos.getPath(mkdirdir)))
    {
        rmdirforce(fileos.getPath(mkdirdir));
    }
    fs.mkdirSync(fileos.getPath(mkdirdir));
    fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output"));
    fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output/Lib"));
    fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output/Lib/"+(os.platform() == "win32"?"win32":"x86")));
    fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output/Lib/"+(os.platform() == "win32"?"win32":"x86")+"/"+name+"/"));

    fileos.copyDir(prjpath+"/Lib/"+(os.platform() == "win32"?"win32":"x86")+"/"+name+"/*",mkdirdir+"/_Output/Lib/"+(os.platform() == "win32"?"win32":"x86")+"/"+name+"/");

    if(fs.existsSync(fileos.getPath(prjpath + "/Include/"+ name)))
    {
        fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output/Include"));
        fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output/Include/"+name));
        fileos.copyDir(prjpath + "/Include/"+ name+"/*",mkdirdir+"/_Output/Include/"+name+"/");
    }

    var zipname = mkdirdir+"//"+name+"_"+os.platform()+"_"+version+".zip";
    oszip.zip(zipname,mkdirdir+"/");
    
    updateTo(name,zipname);
    rmdirforce(fileos.getPath(mkdirdir));
}

function updateTo(name,zipname)
{
    try {
        var dependdir = ms.dependpath();
        if(!fs.existsSync(fileos.getPath(dependdir+"/__VGSII___Compile/"+name)))
        {
            fs.mkdirSync(dependdir+"/__VGSII___Compile/"+name);
        }
        if(fs.existsSync(fileos.getPath(dependdir+"/__VGSII___Compile/"+name)))
        {
            fileos.copy(zipname,dependdir+"/__VGSII___Compile/"+name+"/");
        }
    } catch (error) {
        //该操作可能无权限，所有做一个异常处理
    }   
}

function rmdirforce(path)
{
    while(fs.existsSync(fileos.getPath(path)))
    {
        fileos.rmdir(fileos.getPath(path));
    }
}
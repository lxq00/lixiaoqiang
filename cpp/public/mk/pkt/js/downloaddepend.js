const fs = require("fs");
const os = require("os");
const cp = require('child_process');

const ms = require('./mkshared.js');
const log = require('./pktlog.js');
const fileos = require('./fileos.js');
const oszip = require("./oszip.js");
const filever = require("./version.js");

let downloaddepend=module.exports;

const pkgcfg = require(fileos.getPath(ms.currpath()+"/pkgcfg.json"));

downloaddepend.clean = function()
{
     var currpath = ms.currpath();

     log.log("Start Clean Depend Version!");
     var dependcfg = JSON.parse(fs.readFileSync(fileos.getPath(currpath+"/pkgcfg.json")));
    for(var i = 0;i < dependcfg.Depends.length;i ++){
        dependcfg.Depends[i].version = "";
    }

    fs.writeFileSync(fileos.getPath(currpath+"/pkgcfg.json"),JSON.stringify(dependcfg,null,2));
    log.log("Success Clean Depend Version!");
}

downloaddepend.publish = function()
{
     var currpath = ms.currpath();

     log.log("Start Scan And Publish Object!");
     var havedepends = [];
    if(fs.existsSync(fileos.getPath(currpath+"/.vgsiidepends")))
    {
        //havedepends = JSON.parse(fs.readFileSync(fileos.getPath(currpath+"/.vgsiidepends")));
    }    
    var dependcfg = JSON.parse(fs.readFileSync(fileos.getPath(currpath+"/pkgcfg.json")));
    for(var i = 0;i < dependcfg.Depends.length;i ++){
        dependcfg.Depends[i].version = parseFileVersion(havedepends[dependcfg.Depends[i].name],dependcfg.Depends[i].name);
    }

    fs.writeFileSync(fileos.getPath(currpath+"/pkgcfg.json"),JSON.stringify(dependcfg,null,2));
    log.log("Success Scan And Publish Object!");
}

function parseFileVersion(filename,name)
{
    if(!filename || !name){return "";}

    var exfilename=name+"_"+os.platform()+"_";
    var varex = filename.substring(exfilename.length);
    var varend = varex.indexOf(".zip");
    if(varend == -1)
    {
        return "";
    }
    var verstr = varex.substring(0,varend);
    return verstr;
}

downloaddepend.download = function()
{
    var currpath = ms.currpath();

    var havedepends = {};
    if(fs.existsSync(fileos.getPath(currpath+"/.vgsiidepends")))
    {
        havedepends = JSON.parse(fs.readFileSync(fileos.getPath(currpath+"/.vgsiidepends")));
    }

    var depends = null;
    if(pkgcfg && pkgcfg.Depends)
    {
        depends = pkgcfg.Depends;
    }

    log.log("Start Scan And Download Depend!");
    if(depends)
    {
        for(var i = 0;i < depends.length;i ++)
        {
            downloadAndUnzip(depends[i].name,depends[i].version,havedepends);
        }
    }
    log.log("Success Scan And Download Depend!");
    fs.writeFileSync(fileos.getPath(currpath+"/.vgsiidepends"),JSON.stringify(havedepends,null,2));
}

function downloadAndUnzip(name,version,havedepends)
{
    var verfilename = getVersionFileName(ms.dependpath() + "/__VGSII___Compile",name,version);
    if(verfilename == "")
    {
        return;
    }
    if(havedepends[name] && havedepends[name] == verfilename)
    {
        //return;
    }
    havedepends[name] = verfilename;
    log.log("Downloading:"+verfilename);

    oszip.unzip(ms.dependpath() + "/__VGSII___Compile/"+name+"/"+verfilename,ms.currpath());
}

function getVersionFromFileName(filename,name)
{
    var exfilename=name+"_"+os.platform()+"_";
    var varex = filename.substring(exfilename.length);
    var varend = varex.indexOf("_");
    if(varend == -1)
    {
        return "";
    }
    var verstr = varex.substring(0,varend);
    return verstr;
}

function getVersionFileName(publishdir,name,version)
{
    if(!fs.existsSync(fileos.getPath(publishdir+"/"+ name)))
    {
        return "";
    }
    files = fs.readdirSync(fileos.getPath(publishdir + "/"+ name));
    
    var maxfilename="";
    var maxfileversion="";
    for(var i = 0;i < files.length;i ++)
    {
        if(files[i].indexOf(os.platform()) != -1 && files[i].indexOf(".zip") != -1)
        {
            if(version != "" && files[i].indexOf(version) != -1) 
            {
                return files[i];
            }
            else 
            {
                var fileverstr = getVersionFromFileName(files[i],name);
                var ver1arry = filever.buildFullVersion(fileverstr);
                if(ver1arry[0] >= 4){//老版本不支持4.X的更新
                    continue;
                }
                
                if(maxfilename == "" || filever.cmpVersion(fileverstr,maxfileversion) > 0 || (filever.cmpVersion(fileverstr,maxfileversion) == 0 && files[i] > maxfilename))
                {
                    maxfilename = files[i];
                    maxfileversion= fileverstr;
                }
            }
        }			
    }
    return maxfilename;
}

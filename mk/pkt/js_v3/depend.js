const fs = require("fs");
const os = require("os");
const pr = require('process');
const cp = require('child_process');
const path = require('path');
const fileos = require("../js/fileos.js");
const log = require("../js/pktlog.js");
const ms = require('../js/mkshared.js');
const oszip = require("../js/oszip.js");
const Version = require('./version.js');
const Nuget = require('../js_v4/nuget.js');
const Sln = require('../js_v4/sln.js');

module.exports = class Depend {
    about(){
        log.printdebuginfo(2,"默认/[download/d] [prjname]","下载更新依赖文件");
        log.printdebuginfo(2,"[publish/p]","刷新当前依赖文件到配置文件");
        log.printdebuginfo(2,"[clean/c]","从配置文件中清除当前依赖文件");
        log.printdebuginfo(2,"[reset/r]","从配置文件中重构当前依赖文件");
        log.printdebuginfo(2,"[resetnuget/rn]","从配置文件中重构Nuget版本依赖文件");
    }

    doobj(args){  
        if(!args[1] || args[1] == "" || args[1].toLocaleLowerCase() == "download" || args[1].toLocaleLowerCase() == "d"){
            this._downloadDepend(args[2]?args[2]:"");
        }
        else if(args[1].toLocaleLowerCase() == "publish" || args[1].toLocaleLowerCase() == "p"){
            this._publishDepend();
        }
        else if(args[1].toLocaleLowerCase() == "clean" || args[1].toLocaleLowerCase() == "c"){
            this._cleanDepend();
        }
        else if(args[1].toLocaleLowerCase() == "reset" || args[1].toLocaleLowerCase() == "r"){
            this._resetDepend();
        }
        else if(args[1].toLocaleLowerCase() == "resetnuget" || args[1].toLocaleLowerCase() == "rn"){
            this._resetDependNuget();
        }
        var currpath = pr.cwd();  
        if(fs.existsSync(fileos.getPath(currpath+"/.vgsiidepends"))){
            fileos.rmfile(fileos.getPath(currpath+"/.vgsiidepends"));
        }
    }
    _publishDepend(){
        var currpath = pr.cwd();  

        log.log("Start Scan And Update Depend Version !");
        var dependcfg = JSON.parse(fs.readFileSync(fileos.getPath(currpath+"/pkgcfg.json")));
        var versionarray = new Version()._buildFullVersion(dependcfg.Version);
        if(versionarray[0] >= 4){
            log.log("Not Suport");
            return;
        }
        for(var i = 0;i < dependcfg.Depends.length;i ++){
            if(!dependcfg.DependsVersion) dependcfg.DependsVersion = {};
            if(!dependcfg.DependsVersion[dependcfg.Depends[i].name]) dependcfg.DependsVersion[dependcfg.Depends[i].name] = {};

            dependcfg.DependsVersion[dependcfg.Depends[i].name][os.platform()] = this._getVersionFileName(ms.dependpath() + "/__VGSII___Compile",dependcfg.Depends[i].name,dependcfg.Depends[i].version,false);
        }

        fs.writeFileSync(fileos.getPath(currpath+"/pkgcfg.json"),JSON.stringify(dependcfg,null,2));
        log.log("Success Scan And Update Depend Version !");
    }
    _cleanDepend(){
        var currpath = pr.cwd();  

        log.log("Start Scan And Clean Depend Version!");
        var dependcfg = JSON.parse(fs.readFileSync(fileos.getPath(currpath+"/pkgcfg.json")));
        var versionarray = new Version()._buildFullVersion(dependcfg.Version);
        if(versionarray[0] >= 4){
            log.log("Not Suport");
            return;
        }
        for(var i = 0;i < dependcfg.Depends.length;i ++){
            if(!dependcfg.DependsVersion) continue;
            if(!dependcfg.DependsVersion[dependcfg.Depends[i].name]) continue;

            dependcfg.DependsVersion[dependcfg.Depends[i].name][os.platform()] = "";
        }

        fs.writeFileSync(fileos.getPath(currpath+"/pkgcfg.json"),JSON.stringify(dependcfg,null,2));

        log.log("Success Scan And Clean Depend Version!");
    }
    _resetDepend(){
        var currpath = pr.cwd();  

        log.log("Start Scan And Reset Depend Version!");
        var dependcfg = JSON.parse(fs.readFileSync(fileos.getPath(currpath+"/pkgcfg.json")));
        var versionarray = new Version()._buildFullVersion(dependcfg.Version);
        if(versionarray[0] >= 4){
            log.log("Not Suport");
            return;
        }
        for(var i = 0;i < dependcfg.Depends.length;i ++){
            dependcfg.Depends[i].version = "";          
        }

        fs.writeFileSync(fileos.getPath(currpath+"/pkgcfg.json"),JSON.stringify(dependcfg,null,2));

        log.log("Success Scan And Reset Depend Version!");
    }    
    _insertDpendTargetIntuProject(filename,currpath,prjpath){
            var newfiledata = fs.readFileSync(filename).toString(); 
            var dependstring = `<Import Project="$(ProjectDir)\packages-depend.targets" />`;
            var datapos = newfiledata.indexOf(dependstring);
            if(datapos == -1){
                 var insretpos = newfiledata.indexOf(`<ImportGroup Label="ExtensionTargets">`);
                if(insretpos == -1){
                    return;
                }
                newfiledata = newfiledata.substring(0,insretpos) + dependstring + "\r\n" + newfiledata.substring(insretpos);
            }

            fs.writeFileSync(filename,newfiledata);
        }
        _resetVcProject(currpath,scanpath) {
            var files = fs.readdirSync(scanpath);
            for (var i = 0; i < files.length; i++) {
                if (files[i].substring(0, 1) == "." || files[i].toLocaleLowerCase() == "_output" || files[i].toLocaleLowerCase() == "__compile" || 
                        files[i].toLocaleLowerCase() == "include" || files[i].toLocaleLowerCase() == "lib" || files[i].toLocaleLowerCase() == "mk" || files[i].toLocaleLowerCase() == "bin") {
                    continue;
                }
                var info = fs.statSync(fileos.getPath(scanpath + "/" + files[i]));
                if (info.isDirectory()) {
                    this._resetVcProject(currpath,scanpath + "/" + files[i],files[i]);
                    continue;
                }
                if(path.parse(files[i]).ext.toLocaleLowerCase() != ".vcxproj"){
                    continue;
                }

                this._insertDpendTargetIntuProject(scanpath+"/"+files[i],currpath,scanpath);
                if(fs.existsSync(fileos.getPath(scanpath+"/packages-depend.targets"))){
                    continue;
                }
                fileos.copy(`${__dirname}/../nugettmp/packages-depend.targets`,scanpath+"\\");
            }
        }
    _resetDependNuget(){   
        var currpath = pr.cwd(); 
        log.log("Start Scan And Reset Depend For Nuget Version!");

        var dependcfg = JSON.parse(fs.readFileSync(fileos.getPath(currpath+"/pkgcfg.json")));
        var versionarray = new Version()._buildFullVersion(dependcfg.Version);        
        this._resetVcProject(currpath,currpath);
        dependcfg.Version = "4.0";
        dependcfg.Depends = new Array();
        fs.writeFileSync(fileos.getPath(currpath+"/pkgcfg.json"),JSON.stringify(dependcfg,null,2));
        

        log.log("Success Scan And Reset Depend For Nuget Version!");
    }
    _getdefaultsolution(currpath){
       if(!fs.existsSync(fileos.getPath(currpath + "/pkgcfg.json"))){
           return null;
       }       
       var pkt = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/pkgcfg.json")));
       var versionarray = new Version()._buildFullVersion(pkt.Version);
        if(versionarray[0] < 3) {return null;}

       return pkt.Solution;
    }
    _getproject(currpath,projectname){
        if(!fs.existsSync(fileos.getPath(currpath + "/prjcfg.json"))){
            return null;
        } 
        var prj = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/prjcfg.json")));
        var versionarray = new Version()._buildFullVersion(prj.Version);
        if(versionarray[0] < 3 || !prj.Project) {return null;}

        var solution=null;
        for(var i = 0;i <prj.Project.length;i++){
            if(!prj.Project[i].name){continue;}
            if(projectname != "" && prj.Project[i].name.toLocaleLowerCase() != projectname.toLocaleLowerCase()) {continue;}
            return prj.Project[i];
        }
        return null;
    }
    _downloadDepend(prjname,opt){
        var currpath = pr.cwd();  

         log.log("Start Scan And Download Depend File!");

        if(!fs.existsSync(fileos.getPath(currpath+"/pkgcfg.json"))){
             log.log("Not Found Project 'pkgcfg.json' File");
            return;
        }

        const pkgcfg = require(fileos.getPath(currpath+"/pkgcfg.json"));
        var versionarray = new Version()._buildFullVersion(pkgcfg.Version);
        if(versionarray[0] >= 4){
            if(fs.existsSync(fileos.getPath(currpath+"/_Output"))){
                fileos.rmdir(fileos.getPath(currpath+"/_Output"));
            }
            
            var solution="";
            var prj = this._getproject(currpath,prjname);
            if(prj && prj.solution && ((os.platform() == "win32")?prj.solution.win32:prj.solution.x86)){
                solution = prj.solution;
            }
            else{
                solution = this._getdefaultsolution(currpath);
            }
            if(fs.existsSync(fileos.getPath(currpath+"/packages"))){
                fileos.rmdir(fileos.getPath(currpath+"/packages"));
            }
            new Nuget().restore(currpath,solution && solution.win32 ? solution.win32:"");
        }
        else{
            var depends = null;
            if(pkgcfg && pkgcfg.Depends){
                depends = pkgcfg.Depends;
            }
        
            if(depends){
                for(var i = 0;i < depends.length;i ++){
                    if(depends[i].version[os.platform()]){
                        this._downloadAndUnzip(depends[i].name,depends[i].version[os.platform()],currpath,opt && opt.toLocaleLowerCase() == "beta");
                    }else{
                        var dependver = (pkgcfg.DependsVersion && pkgcfg.DependsVersion[pkgcfg.Depends[i].name] && pkgcfg.DependsVersion[pkgcfg.Depends[i].name][os.platform()])?pkgcfg.DependsVersion[pkgcfg.Depends[i].name][os.platform()]:"";
                        this._downloadAndUnzip(depends[i].name,dependver && dependver != "" ? dependver : depends[i].version,currpath,opt && opt.toLocaleLowerCase() == "beta");
                    }
                }
            }
        }  

        if(fs.existsSync(fileos.getPath(currpath+"/Publish.bat")) && fs.existsSync(fileos.getPath(ms.dependpath()+"/__VGSII__GIT_Tool/vgsii_pkt_tools_2.0/Publish.bat"))){
            fileos.copy(ms.dependpath()+"/__VGSII__GIT_Tool/vgsii_pkt_tools_2.0/Publish.bat",fileos.getPath(currpath));
		}
        if(fs.existsSync(fileos.getPath(currpath+"/Publish.sh")) && fs.existsSync(fileos.getPath(ms.dependpath()+"/__VGSII__GIT_Tool/vgsii_pkt_tools_2.0/Publish.sh"))){
            fileos.copy(ms.dependpath()+"/__VGSII__GIT_Tool/vgsii_pkt_tools_2.0/Publish.sh",fileos.getPath(currpath));
		}
        
        log.log("Success Scan And Download Depend File!");
    }

    _downloadAndUnzip(name,version,currpath,needdev)    {
        var verfilename = this._getVersionFileName(ms.dependpath() + "/__VGSII___Compile",name,version,needdev);
        if(verfilename == ""){
            return;
        }
        log.log("Download Depend File ["+name+"] Version ["+verfilename+"]");

        oszip.unzip(ms.dependpath() + "/__VGSII___Compile/"+name+"/"+verfilename,currpath);
    }

    _getVersionFromFileName(filename,name)    {
        var exfilename="";
        if(filename.indexOf(os.platform()) != -1) exfilename = name+"_"+os.platform()+"_";
        else if(filename.indexOf("common".toLocaleLowerCase()) != -1) exfilename = name+"_common_";       
        
        var varex = filename.substring(exfilename.length);
        var varend = varex.indexOf("_");
        if(varend == -1){
            return "";
        }
        var verstr = varex.substring(0,varend);
        return verstr;
    }
   _isDevPublishZip(filename)   {
      var devflag = filename.indexOf("_beta.zip");

      return devflag >= 0; 
   }
    _getVersionFileName(publishdir,name,version,needdev)    {
        if(!fs.existsSync(fileos.getPath(publishdir+"/"+ name))){
            return "";
        }
        var files = fs.readdirSync(fileos.getPath(publishdir + "/"+ name));
        
        var maxfilename="";
        var maxfileversion="";
        for(var i = 0;i < files.length;i ++){
            if((files[i].indexOf(os.platform()) != -1 || files[i].indexOf("common".toLocaleLowerCase()) != -1)&& files[i].indexOf(".zip") != -1){
                if(!needdev && this._isDevPublishZip(files[i])){
                    continue;
                }                
                if(!this._comdependversion(files[i],name,version)){
                    continue;
                }
                var fileverstr = this._getVersionFromFileName(files[i],name);
                if(maxfilename == "" || this._cmpVersion(fileverstr,maxfileversion) > 0 || (this._cmpVersion(fileverstr,maxfileversion) == 0 && files[i] > maxfilename)){
                    maxfilename = files[i];
                    maxfileversion= fileverstr;
                }
            }			
        }
        return maxfilename;
    }
    _comdependversion(filename,name,version){
        if(!version || version == "" || typeof(version) != "string") version = "<4";
        var fileverstr = this._getVersionFromFileName(filename,name);
        if(version[0] == "~" || version[0] == "<" || version[0] == ">"){
            var dversion = version.substring(1);
            var versionobj = new Version();
            var ver2arry = versionobj._buildFullVersion(dversion);
            var ver1arry = versionobj._buildFullVersion(fileverstr);
            if(version[0] == "~" && ver1arry[0] == ver2arry[0] && ver1arry[1] == ver2arry[1]){
                return true;
            }else if(version[0] == "<" && (ver1arry[0] < ver2arry[0] ||(ver1arry[0] == ver2arry[0] && ver1arry[1] < ver2arry[1]))){
                return true;
            }else if(version[0] == ">" && (ver1arry[0] > ver2arry[0] ||(ver1arry[0] == ver2arry[0] && ver1arry[1] > ver2arry[1]))){
                return true;
            }
        }else if(filename.indexOf(version) != -1){
            return true;
        }

        return false;
    }
    _cmpVersion(ver1,ver2) {
        var version = new Version();
        var ver1arry = version._buildFullVersion(ver1);
        var ver2arry = version._buildFullVersion(ver2);

        for(var i = 0;i < 3 ;i ++){
            if(ver1arry[i] > ver2arry[i]){
                return 1;
            }
            else if(ver1arry[i] < ver2arry[i]){
                return -1;
            }
        }

        return 0;
    }
}

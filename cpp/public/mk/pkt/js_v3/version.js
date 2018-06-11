const fs = require("fs");
const os = require("os");
const pr = require('process');
const cp = require('child_process');
const syspath = require("path");
const fileos = require('../js/fileos.js');
const log = require("../js/pktlog.js");
const Event = require("../js_v3/event.js");

module.exports = class Version {
    about(){
        log.printdebuginfo(2,"默认","更新发布方式的版本号");
        log.printdebuginfo(2,"[get/g]  [prjname]","更新开发方式的版本号，第三位将为git版本");
    }
    doobj(args){       
         
        log.log("Start Scan Version File！");

        var currpath =  pr.cwd();    

        var islib = false;
        if(!args[1]  || (args[1] && (args[1].toLocaleLowerCase() == "get" || args[1].toLocaleLowerCase() == "g"))){
            var prj = this._getproject(currpath,args[2]);
            if(prj && prj.type){
                islib = (prj.type && (prj.type.toLocaleLowerCase() == "lib" || prj.type.toLocaleLowerCase() == "bin"|| 
            prj.type.toLocaleLowerCase() == "bin|lib" || prj.type.toLocaleLowerCase() == "lib|bin" || prj.type.toLocaleLowerCase() == "output"));
            }
        }

        log.log("Start Scan WinVersion.tpl And Update Version");            
        this._scanAndUpdateVersion(currpath,currpath,islib);        
        log.log("Success Scan WinVersion.tpl And Update Version");
    }
    version(lib,prjname,currpath){
        var currpath = currpath ? currpath: pr.cwd();
        var versionstr = this._getVersion(currpath,prjname);
        var suportgitversion = true;
        {
            if(fs.existsSync(fileos.getPath(currpath + "/pkgcfg.json"))){
                var dependcfg = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/pkgcfg.json")));
                var versionarray = this._buildFullVersion(dependcfg.Version);
                suportgitversion = versionarray[0] != 4;
            }   
        }
        
        if(lib && suportgitversion)
            versionstr = this._getGitVersion(versionstr,currpath);
        return versionstr;
    }
    versionString(lib,prjname,currpath){
        var currpath = currpath ? currpath: pr.cwd();
        var version = this.version(lib,prjname,currpath);

        return version.productVersion;
    }
    os(prjname,currpath){
        var currpath = currpath ? currpath: pr.cwd();
        var versioninfo = this._getversionobject(currpath,prjname);

        return versioninfo.os;
    }   
    _buildFullVersion(intputver){
        var inputverarry = new Array();
        inputverarry = intputver.split(".");
        if(inputverarry.length < 1){
            inputverarry[0] = "0";
        }
        if(inputverarry.length < 2){
            inputverarry[1] = "0";
        }
        if(inputverarry.length < 3){
            inputverarry[2] = "0";
        } 
        for(var i = 0;i < 3;i ++) {
            inputverarry[i] = parseInt(inputverarry[i]);
        }

        return inputverarry;
    }
    _getversionobject(currpath,prjname){
        var versioninfo = {};
        if(prjname && prjname != "" && fs.existsSync(fileos.getPath(currpath + "/_Output/Include/"+prjname+"/Version.tpl"))){
            versioninfo = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/_Output/Include/"+prjname+"/Version.tpl")));
        }
        else if(prjname && prjname != "" && fs.existsSync(fileos.getPath(currpath + "/Include/"+prjname+"/Version.tpl"))){
            versioninfo = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/Include/"+prjname+"/Version.tpl")));
        }
        else if(prjname && prjname != "" && fs.existsSync(fileos.getPath(currpath + "/"+prjname+"/Version.tpl"))){
            versioninfo = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/"+prjname+"/Version.tpl")));
        }
        else if(prjname && prjname != "" && fs.existsSync(fileos.getPath(currpath + "/Src/"+prjname+"/Version.tpl"))){
            versioninfo = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/Src/"+prjname+"/Version.tpl")));
        }
        else if(fs.existsSync(fileos.getPath(currpath + "/Version.tpl"))){
            versioninfo = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/Version.tpl")));
        }

        return versioninfo;
    }
    _getVersion(currpath,prjname){
        var versioninfo = this._getversionobject(currpath,prjname);

        if(!versioninfo.productVersion || versioninfo.productVersion == "$ProductVersion$") versioninfo.productVersion = "1.0.0";
        if(!versioninfo.productVersionAlias || versioninfo.productVersionAlias == "$productVersionStage$") versioninfo.productVersionAlias = "";

        //versioninfo.productVersion = this._buildFullVersion(versioninfo.productVersion);

        return versioninfo;
    }

    _getGitVersion(versioninfo,currpath){
        function getGitCommitVersion(currpath,event){
            var gitverdata = "";
            if (os.platform() == "win32") {            
                var gitpath = "";
                if(event && event.win32 && event.win32.git){
                    gitpath = event.win32.git;
                }
                if(gitpath == "") {
                    return 0;
                }
                var cmd  = "call \""+gitpath+"/bin/sh.exe\" "+__dirname + "\\..\\sh\\gitversion.sh";
                gitverdata = cp.execSync(fileos.getPath(cmd));
            }
            else if (os.platform() == "linux") {
                var cmd  = "\""+__dirname+"/../sh/gitversion.sh\"";
               gitverdata =  cp.execSync(fileos.getPath(cmd));
            }
            var version = JSON.parse(gitverdata.toString());
            return version.gitver;
        }
        var event = new Event();
        versioninfo.productVersion = this._buildFullVersion(versioninfo.productVersion);
        versioninfo.productVersion[2] = getGitCommitVersion(currpath,event.event());

        versioninfo.productVersion = versioninfo.productVersion[0]+"."+versioninfo.productVersion[1]+"."+versioninfo.productVersion[2];

        return versioninfo;
    }

    _scanAndUpdateVersion(path,currpath,islib){
        var files = fs.readdirSync(path);
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

            var versioninfo = this.version(islib,syspath.basename(path));
            versioninfo.productVersion = this._buildFullVersion(versioninfo.productVersion);
            var versiontmp = JSON.parse(fileos.readFileSync(fileos.getPath(path + "/WinVersion.tpl")));
            if(versiontmp && versiontmp.filename && versiontmp.productname){
                this._writeVersionini(path,currpath,versioninfo,versiontmp);
                this._writeWinVersion(path,currpath,versioninfo,versiontmp);
            }
        }
        for(var i = 0;i < dirarray.length;i ++){
            this._scanAndUpdateVersion(fileos.getPath(path +"/" +dirarray[i]),currpath,islib);
        }
    }
    _writeWinVersion(path,currpath,versioninfo,versiontmp){
        var verdata = fileos.readFileSync(fileos.getPath(__dirname + "/../tpl/versionrc_v3.tpl"));

        var verdata = this._replaceVersionToTmp(verdata,versioninfo,versiontmp);

        fileos.writeFileSync(fileos.getPath(path + "/Version.rc"), verdata);
    }

    _writeVersionini(path,currpath,versioninfo,versiontmp){
        var verdata  = fs.readFileSync(fileos.getPath(__dirname + "/../tpl/version.inl_v3.tpl"),"utf-8");
        verdata = this._replaceVersionToTmp(verdata,versioninfo,versiontmp);
        fs.writeFileSync(fileos.getPath(path + "/version.inl"), verdata);
    }

    _replaceVersionToTmp(verdata,versioninfo,versiontmp){
        while(verdata.indexOf("{major}") > -1) {
            verdata = verdata.replace("{major}",versioninfo.productVersion[0]);
        }
        while(verdata.indexOf("{minor}") > -1){
            verdata = verdata.replace("{minor}",versioninfo.productVersion[1]);
        }
        while(verdata.indexOf("{build}") > -1){
            verdata = verdata.replace("{build}",versioninfo.productVersion[2]);
        }
        while(verdata.indexOf("{filename}") > -1) {
            verdata = verdata.replace("{filename}",versiontmp.filename);
        }
        while(verdata.indexOf("{versionalias}") > -1) {
            verdata = verdata.replace("{versionalias}",versioninfo.productVersionAlias);
        }
        while(verdata.indexOf("{productname}") > -1) {
            verdata = verdata.replace("{productname}",versiontmp.productname);
        }
        while(verdata.indexOf("{year}") > -1) {
            verdata = verdata.replace("{year}",fileos.date("yyyy"));
        }
        return verdata;
    }

    _getproject(currpath,name){
        if(!fs.existsSync(fileos.getPath(currpath + "/prjcfg.json"))){
            return null;
        } 
        var prj = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/prjcfg.json")));
        var versionarray = this._buildFullVersion(prj.Version);
       if(versionarray[0] < 3 || !prj.Project){
            return null;
       }

        for(var i = 0;i <prj.Project.length;i++){
            if(!prj.Project[i].name){continue;}
            if(!name || name.toLocaleLowerCase() == "default") {
                name = prj.Project[i].name;
                return prj.Project[i];
            }
            if(prj.Project[i].name.toLocaleLowerCase() != name.toLocaleLowerCase()) {continue;}
            return prj.Project[i];
        }

        return null;
    }
}
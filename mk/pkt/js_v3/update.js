const fs = require("fs");
const os = require("os");
const pr = require('process');
const cp = require('child_process');
const fileos = require('../js/fileos.js');
const log = require("../js/pktlog.js");
const ms = require('../js/mkshared.js');
const oszip = require("../js/oszip.js");
const Version = require("./version.js");

module.exports = class Update {
    about(){
        log.printdebuginfo(2,"[Output/o] [beta/b]","发布_Output目录下Beta版本的依赖库文件");
        log.printdebuginfo(2,"[Lib/l] [beta/b]","发布Lib目录下Beta版本的依赖库文件");
        log.printdebuginfo(2,"[Bin/b] [beta/b]","发布Bin目录下Beta版本的依赖库文件");
        log.printdebuginfo(2,"[clean/c]","删除Beta版本的依赖库文件");
    }
    doobj(args){       
        if(!args[1]) {
            log.log("参数错误");
            return;
        }
        var updatepath = new Array();
        updatepath[0] = {};

        var beta = false;
        if(args[1].toLocaleLowerCase() == "output" || args[1].toLocaleLowerCase() == "o"){            
            updatepath[0].includepath = "_Output/Include";
            updatepath[0].libpath = "_Output/Lib";
        }
        else if(args[1].toLocaleLowerCase() == "lib" || args[1].toLocaleLowerCase() == "l"){
            updatepath[0].includepath = "Include";
            updatepath[0].libpath = "Lib";
        }
        else if(args[1].toLocaleLowerCase() == "bin" || args[1].toLocaleLowerCase() == "b"){
            updatepath[0].includepath = "Bin/"+(os.platform() == "win32"?"win32":"x86");
        }
        else if(args[1].toLocaleLowerCase() == "bin|lib" || args[1].toLocaleLowerCase() == "lib|bin"){
            updatepath[1] = {};

            updatepath[0].binpath = "Bin/"+(os.platform() == "win32"?"win32":"x86");
            updatepath[1].includepath = "Include";
            updatepath[1].libpath = "Lib";
        }

        else if(args[1].toLocaleLowerCase() == "clean" || args[1].toLocaleLowerCase() == "c"){
            this._cleanBetaPath();
            return;
        }
        else{
            log.log("Update.js param Error");
            return;
        }
        if(args[2] && (args[2].toLocaleLowerCase() == "beta" || args[2].toLocaleLowerCase() == "b")){
            beta = true;
        }

        for(var i = 0;i < updatepath.length;i ++){
            log.log("Start Scan And Update "+(updatepath[i].binpath ? updatepath[i].binpath : updatepath[i].includepath)+" beta:"+beta);
            this._updatePath(updatepath[i].binpath,updatepath[i].includepath,updatepath[i].libpath,beta);        
            log.log("Success Scan And Update "+(updatepath[i].binpath ? updatepath[i].binpath : updatepath[i].includepath)+" beta:"+beta);
        }
    }

    _cleanBetaPath(){
        if(!fs.existsSync(fileos.getPath(ms.dependpath() + "/__VGSII___Compile/"))){
            return;
        }
        log.log("Start Scan And Clean Beta Packge");
        var files = fs.readdirSync(fileos.getPath(ms.dependpath() + "/__VGSII___Compile"));
        for(var i = 0;i < files.length;i ++){
            if(!fs.existsSync(fileos.getPath(ms.dependpath() + "/__VGSII___Compile/"+files[i]))){
                continue;
            }
            var pktfiles = fs.readdirSync(ms.dependpath() + "/__VGSII___Compile/"+files[i]);
            for(var j = 0;j < pktfiles.length;j ++){
                var devflag = pktfiles[j].indexOf("_beta.zip");
                if(devflag == -1){
                    continue;
                }
                try {
                     fileos.rmfile(ms.dependpath() + "/__VGSII___Compile/"+files[i]+"/"+pktfiles[j]);
                } catch (error) {
                    
                }               
            }            
        }
        log.log("Success Scan And Clean Beta Packge");
    }
    _getprojectname(){
        var flag = "vgsii_";
        var name = log.getProjectName();
        var pos = name.indexOf(flag);
        if(pos == -1){
            return name;
        }

        return name.substr(pos+flag.length);
    }
    _updatePath(binpath,includepath,libpath,beta){
         var currpath =  pr.cwd();
         var excule = [];
         if(fs.existsSync(fileos.getPath(currpath + "/config.json"))){
            var config = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/config.json")));
            if(config.Version && config.Version == "3.0" && config.Package && config.Package.Exclude){
                excule = config.Package.Exclude;
            }
        }

        if(binpath){
            this._zipUpdateDirAndUpdate(this._getprojectname(),binpath,null,null,currpath,beta);
        }
        var haveupdate = new Array();
        if(includepath && fs.existsSync(fileos.getPath(currpath+"/" +includepath))){
            var files = fs.readdirSync(fileos.getPath(currpath+"/" +includepath));
            for(var i = 0;i < files.length;i ++) {
                haveupdate[files[i].toLocaleLowerCase()] = "1";
                if(this._isInExclude(files[i],excule)) {
                    continue;
                }
                log.log("Start Package And Update "+files[i]);
                this._zipUpdateDirAndUpdate(files[i],null,includepath,libpath,currpath,beta);
            }
        }
        if(libpath && fs.existsSync(fileos.getPath(currpath+"/" +libpath+"/"+(os.platform() == "win32"?"win32":"x86")))){
            var files = fs.readdirSync(fileos.getPath(currpath+"/" +libpath+"/"+(os.platform() == "win32"?"win32":"x86")));
            for(var i = 0;i < files.length;i ++) {
                if(haveupdate[files[i].toLocaleLowerCase()]) continue;
                 haveupdate[files[i].toLocaleLowerCase()] = "1";
                 if(this._isInExclude(files[i],excule)) {
                    continue;
                }
                log.log("Start Package And Update "+files[i]);
                this._zipUpdateDirAndUpdate(files[i],null,includepath,libpath,currpath,beta);
            }
        }
    }
    _isInExclude(name,Exclude){
        if(Exclude){
            for(var i = 0;i < Exclude.length;i ++){
                if(name.toLocaleLowerCase() == Exclude[i].toLocaleLowerCase()){
                    return true;
                }
            }
        }

        return false;
    }
    _zipUpdateDirAndUpdate(name,binpath,includepath,libpath,currpath,beta)    {       

        var mkdirdir = currpath + "/__" + name + "__tmp";
        if(fs.existsSync(fileos.getPath(mkdirdir)))
        {
            fileos.rmdirforce(fileos.getPath(mkdirdir));
        }
        fs.mkdirSync(fileos.getPath(mkdirdir));

        if(binpath && fs.existsSync(fileos.getPath(currpath+"/"+binpath))){
            fs.mkdirSync(fileos.getPath(mkdirdir+"/Bin"));
            fs.mkdirSync(fileos.getPath(mkdirdir+"/Bin/"+(os.platform() == "win32"?"win32":"x86")));

            fileos.copyDir(currpath+"/"+binpath+"/*",mkdirdir+"/Bin/"+(os.platform() == "win32"?"win32":"x86")+"/");
        }

        if(includepath && fs.existsSync(fileos.getPath(currpath+"/"+includepath+"/"+name))){
            fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output"));
            fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output/Include"));
            fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output/Include/"+name));

            fileos.copyDir(currpath+"/"+includepath+"/"+name+"/*",mkdirdir+"/_Output/Include/"+name+"/");
        }
        if(libpath && fs.existsSync(fileos.getPath(currpath + "/"+libpath+"/"+(os.platform() == "win32"?"win32":"x86")+"/"+ name)))
        {
            if(!fs.existsSync(mkdirdir+"/_Output")) fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output"));
            fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output/Lib"));
            fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output/Lib/"+(os.platform() == "win32"?"win32":"x86")));
            fs.mkdirSync(fileos.getPath(mkdirdir+"/_Output/Lib/"+(os.platform() == "win32"?"win32":"x86")+"/"+ name));
            fileos.copyDir(currpath + "/"+libpath+"/"+(os.platform() == "win32"?"win32":"x86")+"/"+ name+"/*",mkdirdir+"/_Output/Lib/"+(os.platform() == "win32"?"win32":"x86")+"/"+ name+"/");
        }

        var version = new Version().versionString(true,name);
        var osstring = new Version().os(name);
        osstring = osstring && osstring.toLocaleLowerCase() == "common" ? osstring.toLocaleLowerCase() : os.platform();

        var zipname = mkdirdir+"//"+name+"_"+osstring+"_"+version+(beta?"_beta":"")+"_"+fileos.date("yyyyMMdd")+".zip";
        oszip.zip(zipname,mkdirdir+"/");
        
        this._updateTo(name,zipname);
        fileos.rmdirforce(fileos.getPath(mkdirdir));
    }

    _updateTo(name,zipname){
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

}
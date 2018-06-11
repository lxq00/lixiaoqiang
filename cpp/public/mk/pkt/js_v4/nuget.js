const fs = require("fs");
const os = require("os");
const cp = require('child_process');
const pr = require('process');
const path = require('path');
const fileos = require('../js/fileos.js');
const Version = require("../js_v3/version.js");
const Event = require("../js_v3/event.js")
const oszip = require("../js/oszip.js");
const log = require("../js/pktlog.js");
const Sln = require('../js_v4/sln.js');

var _nugetServerUrl = "http://nuget.cd.xunmei.com/nuget/cpp/package/{name}/{version}";

module.exports = class Nuget {
    restore(currpath,slnfile){
        /*
       if(os.platform() == "win32"){
           var nugetpath = new Event().event().nuget;
           if(nugetpath&&nugetpath != ""){
                var cmd = `call "${nugetpath}" restore "${slnfile}"`;
                cp.execSync(cmd);
                return;
           }
        }*/
        this._restore(currpath,currpath);             
    }
    _restore(currpath,prjpath,name) {
        currpath = currpath ? currpath : pr.cwd();
        prjpath = prjpath ? prjpath : pr.cwd();
        var files = fs.readdirSync(prjpath);
        for (var i = 0; i < files.length; i++) {
            if (files[i].substring(0, 1) == "." || files[i] == "_Output" || files[i] == "__Compile" || files[i] == "Include" || files[i] == "Lib" || files[i] == "mk") {
                continue;
            }
            var info = fs.statSync(fileos.getPath(prjpath + "/" + files[i]));
            if (info.isDirectory()) {
                this._restore(currpath,prjpath + "/" + files[i],files[i]);
                continue;
            }
            else if (files[i] != "packages.config") {
                continue;
            }
            
            this._downloadNuget(currpath,prjpath + "/" + files[i]);
            var mkfile = this._findMkfile(path.parse(prjpath).base,prjpath);
            if(mkfile != ""){
                this._getmkfilenuget(currpath,prjpath,mkfile);
            }
        }
    }
    
     

    _downloadPackeg(currpath,nuget,todir,downloadfile){
        if(fs.existsSync(fileos.getPath(downloadfile))){
            return;
        }
        var url = _nugetServerUrl.replace("{name}",nuget.name);
        url = url.replace("{version}",nuget.version);

        var cmd = "";
        if(os.platform() == "win32"){
            cmd = `call ${__dirname}/../tool/wget.exe -q -O ${downloadfile} ${url}`;
        }
        else{
            cmd = `wget -q -O ${downloadfile} ${url}`;
        }
        cp.execSync(cmd);
        log.log(`Start Download Nuget Package ${nuget.name}.${nuget.version}.zip`);

        if(fs.existsSync(fileos.getPath(downloadfile))){
            oszip.unzip(downloadfile,todir);
        }
    }

    _downloadNuget(currpath,prjfilename){
        if(!fs.existsSync(fileos.getPath(`${currpath}/packages`))){
            fs.mkdirSync(fileos.getPath(`${currpath}/packages`));
        }
        var nugetpkg = new Sln()._parseNugetPacket(fileos.getPath(prjfilename));

        for(var i = 0;i < nugetpkg.length;i ++){
            if(nugetpkg[i].version == "") continue;

            var todir = fileos.getPath(`${currpath}/packages/${nugetpkg[i].name}.${nugetpkg[i].version}`);
            var downloadfile=fileos.getPath(`${todir}/${nugetpkg[i].name}.${nugetpkg[i].version}.nupkg`);
            
            var extmkfile = fileos.getPath(`${todir}/build/native/${nugetpkg[i].name}.mk`);

            if(fs.existsSync(todir)){
                continue;
            }
            fs.mkdirSync(todir);
            this._downloadPackeg(currpath,nugetpkg[i],todir,downloadfile);
            
            if(fs.existsSync(extmkfile)){
                var filedata = fileos.readFileSync(extmkfile).toString();
                while(filedata.indexOf("{NugetPath}") != -1){
                    filedata = filedata.replace("{NugetPath}",todir);
                }
                fileos.writeFileSync(extmkfile,filedata);
            }
        }
    }
    _findMkfile(name,prjpath){
        var namemkfile = "";
        var defaultmkfile="";

        var files = fs.readdirSync(prjpath);
        for (var i = 0; i < files.length; i++) {
            if (files[i].substring(0, 1) == "." || files[i] == "_Output" || files[i] == "__Compile" || files[i] == "Include" || files[i] == "Lib" || files[i] == "mk") {
                continue;
            }
            var info = fs.statSync(fileos.getPath(prjpath + "/" + files[i]));
            if (info.isDirectory()) {
                continue;
            }
            if(path.parse(files[i]).ext.toLocaleLowerCase() != ".mk"){
                continue;
            }

            defaultmkfile = `${prjpath}/${files[i]}`;
            if(path.parse(files[i]).name.toLocaleLowerCase() == name.toLocaleLowerCase()){
                namemkfile = `${prjpath}/${files[i]}`;
            }
        }

        return namemkfile != "" ? namemkfile : defaultmkfile;
    }
    _parseappfilename(filename){
        var filedata = fileos.readFileSync(filename).toString();
        var namestartpos = filedata.indexOf(`<name>`);
        if(namestartpos == -1){
            return "";
        }
        filedata = filedata.substring(namestartpos+6);
        var nameendpos = filedata.indexOf(`</name>`);
        if(nameendpos == -1){
            return "";
        }
        return filedata.substring(0,nameendpos);
    }
    _getmkfilenuget(currpath,prjpath,mkfile){
        if(!fs.existsSync(fileos.getPath(`${prjpath}/packages-app.config`))){
            return new Array();
        }

        if(this._parseappfilename(`${prjpath}/packages-app.config`).toLocaleLowerCase() != path.parse(mkfile).base.toLocaleLowerCase()){
            return new Array();
        }

        return this._downloadNuget(currpath,`${prjpath}/packages-app.config`);
    }    
}
const fs = require("fs");
const os = require("os");
const pr = require('process');
const cp = require('child_process');
const fileos = require("../js/fileos.js");
const log = require("../js/pktlog.js");
const Version = require("../js_v3/version.js");
const main = require("../js_v3/main.js");
const bale_customize = require("../js_v3/bale_customize.js")

module.exports = class Publish {
    about(){
        log.printdebuginfo(2,"default/[ProjectName] [debug/d] [build/b]","发布指定项目工程");
    }

    doobj(args){  
        var currpath =  pr.cwd();
         var projectname="",debug=false,build=false;

        if(args[1]){
            projectname = args[1].toLocaleLowerCase() == "default" ? "" : args[1];
        }
        if(args[2]){
            debug = args[2].toLocaleLowerCase() == "debug" || args[2].toLocaleLowerCase() == "d";
        }
        if(args[3]){
            build = args[3].toLocaleLowerCase() == "build" || args[3].toLocaleLowerCase() == "b";
        }

        log.log("Start Publish Project ["+projectname+"]");
        var prj = this._getproject(currpath,projectname);
        if(!prj){
            log.log("Publish Error,Not Found Project["+projectname+"]");
            return;
        }

        var version = new Version();
        var veralias = version.version().productVersionAlias;
        var isneedupdate = (prj.type && (prj.type.toLocaleLowerCase() == "lib" || prj.type.toLocaleLowerCase() == "bin"|| 
            prj.type.toLocaleLowerCase() == "bin|lib" || prj.type.toLocaleLowerCase() == "lib|bin" || prj.type.toLocaleLowerCase() == "output"));

        main.doCmdHandle("version",new Array("","get",prj.name));
        main.doCmdHandle("depend",(veralias && veralias != "") ? new Array("","download","beta"):"");
        main.doCmdHandle("compile",new Array("",prj.name,debug?"debug":"release",build?"build":"rebuild"));
        if(isneedupdate){
			if((os.platform() == "win32")) main.doCmdHandle("compile",new Array("",prj.name,!debug?"debug":"release",build?"build":"rebuild"));
            main.doCmdHandle("update",new Array("",prj.type));
         }
         else if(prj.type&& prj.type.toLocaleLowerCase() == "customize"){
            main.doCmdHandle("balecustomize",new Array("",prj.name,debug?"debug":"release"));
         }else{
             main.doCmdHandle("bale",new Array("",prj.name,debug?"debug":"release"));
        }

        log.log("Success Publish Project ["+projectname+"]");
    }

    _getproject(currpath,name){
        if(!fs.existsSync(fileos.getPath(currpath + "/prjcfg.json"))){
            return null;
        } 
        var prj = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/prjcfg.json")));
        var versionarray = new Version()._buildFullVersion(prj.Version);
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
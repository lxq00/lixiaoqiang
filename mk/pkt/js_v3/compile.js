const fs = require("fs");
const os = require("os");
const pr = require('process');
const cp = require('child_process');
const log = require("../js/pktlog.js");
const fileos = require("../js/fileos.js");
const ms = require('../js/mkshared.js');
const Event = require("../js_v3/event.js");
const Version = require('./version.js');

module.exports = class Compile {

about(){
        log.printdebuginfo(2,"default/[ProjectName] [debug/d] [build/b]","编译指定项目工程");
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

        log.log("Start Compile ["+projectname+"] debug="+debug+" build="+build);

        var pktname = this._currgitname(currpath);
        var projectdir="",solution="",beforecmd="";
        var prj = this._getproject(currpath,projectname);
        if(prj && prj.solution && ((os.platform() == "win32")?prj.solution.win32:prj.solution.x86)){
            solution = prj.solution;
			projectdir=prj.dir;
			beforecmd = prj.solutionbefore;
        }
        else{
            solution = this._getdefaultsolution(currpath);
        }

        projectdir = (!projectdir || projectdir == "") ? currpath : fileos.getPath(currpath + "/" + projectdir);
        var outputfile = fileos.getPath(ms.logpath() + "/" +pktname+"_"+ os.platform()+(projectname == "" ? "" : ("_"+projectname)+"_"+(debug?"Debug":"Release"))+"_output.txt");

        var event = new Event();

		this._runCompileBefore(currpath,projectdir,beforecmd);

        if(os.platform() == "win32")  this._compileWin32(event.event(),projectdir,solution,outputfile,debug,!build);
        else this._compilelinux(currpath,event.event(),projectdir,solution,outputfile,debug,!build);

         log.log("Success Compile ["+projectname+"]");
    }

    _getdefaultsolution(currpath){
       if(!fs.existsSync(fileos.getPath(currpath + "/pkgcfg.json"))){
           return null;
       }       
       var pkt = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/pkgcfg.json")));
       var versionarray = new Version()._buildFullVersion(pkt.Version);
       if(versionarray[0] < 3){
            return null;
       }

       return pkt.Solution;
    }
    _getproject(currpath,projectname){
        if(projectname == ""){return null;}
        if(!fs.existsSync(fileos.getPath(currpath + "/prjcfg.json"))){
            return null;
        } 
        var prj = JSON.parse(fs.readFileSync(fileos.getPath(currpath + "/prjcfg.json")));
        var versionarray = new Version()._buildFullVersion(prj.Version);
        if(versionarray[0] < 3 || !prj.Project){
            return null;
        }

        var solution=null;
        for(var i = 0;i <prj.Project.length;i++){
            if(!prj.Project[i].name){continue;}
            if(prj.Project[i].name.toLocaleLowerCase() != projectname.toLocaleLowerCase()) {continue;}
            return prj.Project[i];
        }
        return null;
    }
	_runCompileBefore(currpath,prjpath,before){
		if(!prjpath || !before || before=="") {return;}

		if(!fs.existsSync(fileos.getPath(fileos.getPath(prjpath+"/"+before)))){
			log.log("Compile File ["+fileos.getPath(prjpath+"/"+before)+"] Not Exists");
            return;
        } 
		log.log("Run Compile ["+fileos.getPath(prjpath+"/"+before)+"]");
		var cmd = "";
        if(os.platform() == "win32"){
            cmd = "cd \""+prjpath+"\" && call \""+before+"\"";
        }
        else{
            cmd = "cd \""+prjpath+"\" && /bin/sh \""+before+"\"";
        }
		log.log(cmd);
        cp.execSync(cmd);
	}
    _compileWin32(event,solutionpath,solution,outputfile,debug,rebuild){
        if(!solution || !solution.win32) return;

        if(!event || !event.win32 || !event.win32.msbuild || !fs.existsSync(event.win32.msbuild)){
            log.log("windows compile not find msbuild");
            return;
        }
        if(!fs.existsSync(fileos.getPath(solutionpath + "/" + solution.win32))){
            log.log("windows compile not find solution "+ fileos.getPath(solutionpath + "/" + solution.win32));
            return;
        }

        var param = (rebuild ? "/t:Rebuild":"")+" /p:Configuration="+(debug?"Debug":"Release") + " > \"" + outputfile+"\"";
        var cmd = "cd \""+solutionpath + "\" && call \"" +event.win32.msbuild + "\" \""+ solution.win32+ "\" "+ param;

        log.log(cmd);
        cp.execSync(cmd);
    }
    _compilelinux(currpath,event,solutionpath,solution,outputfile,debug,rebuild){
        if(!solution || !solution.x86) return;
		
		if(currpath != solutionpath){
			fileos.copyDir(fileos.getPath(currpath+"/mk"),solutionpath);
		}

        if(!fs.existsSync(fileos.getPath(solutionpath + "/" + solution.x86))){
            log.log("linux compile not find solution " + solution.x86);
            return;
        }
        
        var cmdstr = "cd "+solutionpath+(rebuild? "&&make clean" : "")+"&&make > "+outputfile;
        log.log(cmdstr);
        cp.execSync(cmdstr);

		if(currpath != solutionpath && fs.existsSync(fileos.getPath(solutionpath+"/mk"))){
			fileos.rmdir(fileos.getPath(solutionpath+"/mk"));
		}
    }
    _currgitname(currpath){
        var name = currpath;
        while(1)        {
            var pos = name.indexOf("\\");
            if(pos <= -1)   pos = name.indexOf("/");
            if(pos <= -1)   break;

            name = name.substring(pos + 1);
        }
        return name;
    }
}
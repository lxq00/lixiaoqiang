const fs = require("fs");
const os = require("os");
const pr = require('process');
const cp = require('child_process');
const fileos = require("../js/fileos.js");
const log = require("../js/pktlog.js");
const ms = require("../js/mkshared.js");
const oszip = require("../js/oszip.js");
const Event = require("../js_v3/event.js");
const Version = require("../js_v3/version.js");
const CopyDll = require("../js_v4/copydll.js");

module.exports = class Bale {
    about(){
        log.printdebuginfo(2,"default/[ProjectName] [debug/d]","打包指定项目工程");
    }

    doobj(args){  
        var currpath =  pr.cwd();
         var projectname="",debug=false;

         if(args[1]){
            projectname = args[1].toLocaleLowerCase() == "default" ? "" : args[1];
        }
        if(args[2]){
            debug = args[2].toLocaleLowerCase() == "debug" || args[2].toLocaleLowerCase() == "d";
        }
        log.log("Start Scan ["+projectname+"] And Bale!");
        this._package(currpath,projectname,debug);
        log.log("Success Scan ["+projectname+"] And Bale!");
    }

    _package(currpath,name,debug){
        //获取项目配合
        var prj = this._getproject(currpath,name);
        if(prj == null){
            log.log("Project ["+name+"] Not Find Project Info");
            return;
        }
        //检测项目输出类型是否支持
        if(!this._checkPrjOutputType(prj)){
            log.log("Project ["+prj.name+"] Output Type ["+prj.type+"] Not Support!");
            return;
        }
        //检测项目工作目录
        if(!this._checkPrjWorkPath(currpath,prj)){
             log.log("Project ["+prj.name+"] 'dir' ["+prj.dir+"] Not Exists");
            return;
        }
        var prjpath = prj.dir ? fileos.getPath(currpath + "/"+prj.dir) : currpath;

        //创建输出目录
        var outputpath = this._createOutputDir(prjpath,prj);
        
        //拷贝动态依赖项目
        this._copydependProject(outputpath,prj,currpath,debug);
        //拷贝公共依赖包
        this._copyPublishInstall(currpath);
        //拷贝输出的exe文件
        this._copyExenameFile(currpath,prjpath,prj,outputpath,debug);
        //拷贝动态依赖文件
        this._copyDynamicDepend(currpath,prjpath,prj,outputpath,debug);
        //拷贝安装目录文件
        this._copyInstallPath(prjpath,prj,outputpath);
        //更新安装文件夹
        this._replaceInstallFile(prjpath,prj,outputpath,debug);

        //进行数字签名
        this._signature(currpath,outputpath,prj,false,"");

        //执行外部命令
        this._extandCmd(currpath,prjpath,prj,outputpath,debug);
        
        //执行setupfactory
        var setupfile = this._runSetupfactory(currpath,prjpath,prj,outputpath);
        if(setupfile){

            this._signature(currpath,outputpath,prj,true,setupfile);

            if(prj.setupfactoryOnly){
                //重新创建文件列表
                this._createOutputDir(prjpath,prj);
                //拷贝安装目录文件
                this._copyInstallPath(prjpath,prj,outputpath);
                //更新安装文件夹
                this._replaceInstallFile(prjpath,prj,outputpath);
            }
            fileos.copy(setupfile,outputpath);            
        }

        var outputfile = fileos.getPath(currpath + "/" + prj.name) + (prj.type.toLocaleLowerCase() == "exe" ? ".exe" : ".zip");
         if(fs.existsSync(fileos.getPath(outputfile))){
             fileos.rmfile(fileos.getPath(outputfile));
         }
        if(prj.type.toLocaleLowerCase() == "exe"){
            if(!setupfile) return;
            fileos.move(setupfile,outputfile);
        }else{
            if(fs.existsSync(outputfile)){
                fileos.rmfile(outputfile);
            }
            this._backuppdb(outputpath,debug);
			this._removeDelete(outputpath,debug);
            oszip.zip(outputfile,outputpath+"\\");
        }
        if(fs.existsSync(setupfile)){
            fileos.rmfile(setupfile);
        }
         if(fs.existsSync(outputpath)){
            fileos.rmdirforce(outputpath);
        }

        if(!fs.existsSync(fileos.getPath(outputfile))){
            log.log("Bale Error, Output File ["+outputfile+"] Not Exists");
            return;
        }
        if(fs.existsSync(fileos.getPath(currpath+"/install_tmp"))){
            fileos.rmdirforce(currpath+"/install_tmp");
        }

        return fileos.getPath(outputfile);
    } 
	_removeDelete(tmpdirname,debug){
		var files = fs.readdirSync(fileos.getPath(tmpdirname));
		for(var i = 0;i < files.length;i ++){
			if(files[i].toLocaleLowerCase() != "pdb" && this._fileIsDirectory(tmpdirname+"//"+files[i])){
				this._removeDelete(tmpdirname+"//"+files[i],debug);
			}
		}

		fileos.rmfile(tmpdirname+"\\*.ilk",false);
		fileos.rmfile(tmpdirname+"\\*.lib",false);
		fileos.rmfile(tmpdirname+"\\*.iobj",false);
		fileos.rmfile(tmpdirname+"\\*.ipdb",false);
		fileos.rmfile(tmpdirname+"\\*.exp",false);
		fileos.rmfile(tmpdirname+"\\*.h",false);
		fileos.rmfile(tmpdirname+"\\*.doc",false);
		fileos.rmfile(tmpdirname+"\\*.docx",false);
		fileos.rmfile(tmpdirname+"\\*.chm",false);

//删除系统保留库
        var syslib = [
            {"name":"msvcp","version":["90","100","110","120"]},
            {"name":"msvcr","version":["90","100","110","120"]},
            {"name":"mfc","version":["90","100","110","120"]},
            {"name":"msvcrt","version":[""]},
        ];
        for(var i = 0;i < syslib.length && !debug;i ++){
            for(var j = 0;j < syslib[i].version.length;j ++){
                fileos.rmfile(tmpdirname+"\\"+syslib[i].name+syslib[i].version[j]+"d.dll");
            }
        }
        
	}
    _extandCmd(currpath,prjpath,prj,outputpath,debug){
        if(!prj.extandCmd || !fs.existsSync(fileos.getPath(prjpath+"/"+prj.extandCmd))) return;

        var version = new Version();
        var ver = version.versionString(prj.type.toLocaleLowerCase() == "lib");

        var cmd = "";
        if(os.platform() == "win32"){
            cmd = "cd \""+prjpath+"\" && call \""+fileos.getPath(prjpath+"/"+prj.extandCmd)+"\" \""+currpath+"\" \""+prjpath+"\" "+prj.name+" "+ver+" \""+outputpath+"\" "+(debug?"debug":"release");
        }
        else{
            cmd = "cd \""+prjpath+"\" && /bin/sh \""+fileos.getPath(prjpath+"/"+prj.extandCmd)+"\" \""+currpath+"\" \""+prjpath+"\" "+prj.name+" "+ver+" \""+outputpath+"\" "+(debug?"debug":"release");
        }

        cp.execSync(cmd);
    }
    _signature(currpath,outputpath,prj,outputfile,outputfilename){
        if(os.platform() != "win32") return;
        if(!prj.signature || prj.signature.length <= 0) return;

        if(outputfile && outputfilename){
            for(var i = 0;i < prj.signature.length;i ++){
                if(prj.signature[i].file && prj.signature[i].file.toLocaleLowerCase() == "{setupfactoryOutputFile}".toLocaleLowerCase()){
                    this._signaturefile(currpath,outputfilename,prj.signature[i].readme);
                    break;
                }
            }
        }else{
            for(var i = 0;i < prj.signature.length;i ++){
                if(prj.signature[i].file && fs.existsSync(fileos.getPath(outputpath+"/"+prj.signature[i].file))){
                    this._signaturefile(currpath,fileos.getPath(outputpath+"/"+prj.signature[i].file),prj.signature[i].readme);
                }
            }
        }
    }
    _signaturefile(currpath,file,readme){
        var cmd = "call \""+fileos.getPath(currpath+"\\mk\\pkt\\tool\\signtool.exe")+"\" sign -t http://timestamp.verisign.com/scripts/timstamp.dll -d \""+(readme?readme:"")+"\" -du http://www.xunmei.com -n \"Xunmei Electronic Technology Co.,Ltd.\" \""+file+"\"";

        try {
            cp.execSync(cmd);
        } catch (error) {            
        }        
    }
    _backuppdb(outputpath,debug){
        if(debug) return;

        var files = fs.readdirSync(fileos.getPath(outputpath));
        for(var i = 0;i < files.length;i ++){
            if(this._fileIsDirectory(fileos.getPath(outputpath+"/"+files[i])))  continue;
            var filetype = files[i].lastIndexOf(".") == -1 ? "":files[i].substring(files[i].lastIndexOf(".") + 1,files[i].length);
            if(filetype.toLocaleLowerCase() != "pdb") continue;

             if(!fs.existsSync(fileos.getPath(outputpath+"/pdb"))){
                fs.mkdirSync(fileos.getPath(outputpath+"/pdb",777));
            }
            
            fileos.copy(fileos.getPath(outputpath+"/"+files[i]),fileos.getPath(outputpath+"/pdb"));
            fileos.rmfile(fileos.getPath(outputpath+"/"+files[i]));
        }
    }
    _runSetupfactory(currpath,prjpath,prj,outputpath){
        if(!prj.setupfactory || !prj.setupfactory.inputdir || !prj.setupfactory.cfg || !prj.setupfactory.outputfile) return;

        if(!fs.existsSync(fileos.getPath(prjpath+"/"+prj.setupfactory.cfg))) return;

        this._udpateSetupFactoryVersion(currpath,prjpath,prj);

        var eventobj = new Event();
        var event = eventobj.event();
        if(!event || !event.win32 || !event.win32.setupfactory || !fs.existsSync(event.win32.setupfactory)){
            log.log("windows not find setupfactory");
            return;
        }

        this._createDir(fileos.getPath(prj.setupfactory.inputdir));
        if(!fs.existsSync(fileos.getPath(prj.setupfactory.inputdir))){
            log.log("setupfactory inputdir ["+fileos.getPath(prj.setupfactory.inputdir)+"]文件夹创建失败");
            return;
        }

        fileos.copyDir(outputpath,prj.setupfactory.inputdir);

        var cmd = "cd "+prjpath+"&& call \""+event.win32.setupfactory+"\" /BUILD \"" +fileos.getPath(prjpath+"/"+prj.setupfactory.cfg) + "\" /CONFIG:Default";
        
        log.log(cmd);
        cp.execSync(cmd);

        fileos.rmdirforce(fileos.getPath(prj.setupfactory.inputdir));

        if(!fs.existsSync(fileos.getPath(prj.setupfactory.outputfile))){
            log.log("setupfactory output file ["+fileos.getPath(prj.setupfactory.outputfile)+"]不存在");
            return;
        } 

        return fileos.getPath(prj.setupfactory.outputfile);
    }
    _udpateSetupFactoryVersion(currpath,prjpath,prj){
        if(!prj.setupfactory || !prj.setupfactory.updateversion) return;

        var version = new Version();
        var ver = version.versionString(prj.type.toLocaleLowerCase() == "lib");

        var setupcfgdata = fileos.readFileSync(fileos.getPath(prjpath + "/"+prj.setupfactory.cfg));

        var versionflag = "<Name>%ProductVer%</Name>";

        var pos = setupcfgdata.indexOf(versionflag);
        if(pos == -1) return;

        var newdata = setupcfgdata.substr(0,pos + versionflag.length);
        var oldverdata = setupcfgdata.substr(pos + versionflag.length,setupcfgdata.length);
        
        var versionendflag = "</Value>";
        var endpos = oldverdata.indexOf(versionendflag);
        if(endpos == -1) return;

        newdata += "\r\n<Value>" + ver + oldverdata.substr(endpos,oldverdata.length);

        fileos.writeFileSync(fileos.getPath(prjpath + "/"+prj.setupfactory.cfg),newdata);
    }
	_copyPublishInstall(currpath){
        if(fs.existsSync(fileos.getPath(ms.dependpath()+"/__VGSII__GIT_Tool/vgsii_pkt_tools_2.0/install"))){
            if(!fs.existsSync(fileos.getPath(currpath+"/install_tmp"))) fs.mkdirSync(fileos.getPath(currpath+"/install_tmp"));
			fileos.copyDir(ms.dependpath()+"/__VGSII__GIT_Tool/vgsii_pkt_tools_2.0/install",fileos.getPath(currpath+"/install_tmp"));
		}
    }
    _replaceInstallFile(prjpath,prj,tmpdirname,debug){
		
        var exename = os.platform() == "win32" ? prj.exename+".exe" : prj.exename;

        var version = new Version();
        var ver = version.versionString(prj.type.toLocaleLowerCase() == "lib");

        var xmldata = "";
        if (fs.existsSync(fileos.getPath(tmpdirname + "/ServerComponent.xml")))
        {
            xmldata = fs.readFileSync(fileos.getPath(tmpdirname + "/ServerComponent.xml"));
        }
        else if (fs.existsSync(fileos.getPath(tmpdirname + "/SDK.xml"))) {
            xmldata = fs.readFileSync(fileos.getPath(tmpdirname + "/SDK.xml"));
        }
        else if (fs.existsSync(fileos.getPath(tmpdirname + "/Decode.xml"))) {
            xmldata = fs.readFileSync(fileos.getPath(tmpdirname + "/Decode.xml"));
        }
        else if (fs.existsSync(fileos.getPath(tmpdirname + "/ComponentType.xml"))) {
            xmldata = fs.readFileSync(fileos.getPath(tmpdirname + "/ComponentType.xml"));
        }

        var date = new Date();
        xmldata = xmldata.toString().replace("{PublishVersion}", ver);
        xmldata = xmldata.replace("{PublishTime}", date.format("yyyy-MM-dd"));
        xmldata = xmldata.replace("{Platform}", os.platform() == "win32" ? "Windows" : (os.platform() == "linux" ? "Linux":""));
        xmldata = xmldata.toString().replace("{exeName}", exename);
        
        if (fs.existsSync(fileos.getPath(tmpdirname + "/ServerComponent.xml"))) {
            fs.writeFileSync(fileos.getPath(tmpdirname + "/ServerComponent.xml"), xmldata);
        }
        else if (fs.existsSync(fileos.getPath(tmpdirname + "/SDK.xml"))) {
            fs.writeFileSync(fileos.getPath(tmpdirname + "/SDK.xml"), xmldata);
        }
        else if (fs.existsSync(fileos.getPath(tmpdirname + "/Decode.xml"))) {
            fs.writeFileSync(fileos.getPath(tmpdirname + "/Decode.xml"), xmldata);
        }
        else if (fs.existsSync(fileos.getPath(tmpdirname + "/ComponentType.xml"))) {
            fs.writeFileSync(fileos.getPath(tmpdirname + "/ComponentType.xml"), xmldata);
        }
        if (fs.existsSync(fileos.getPath(tmpdirname + "/readme.md")))
        {
            var xmldata = fs.readFileSync(fileos.getPath(tmpdirname + "/readme.md"));
            xmldata = xmldata.toString().replace("{PublishVersion}", ver);
            xmldata = xmldata.replace("{PublishTime}", date.format("yyyy年MM月dd日"));
            xmldata = xmldata.replace("{customizereadme}", prj.readme ? prj.readme : "");
            fs.writeFileSync(fileos.getPath(tmpdirname + "/readme.md"), xmldata);
        }
    }
    _isInstallCommuPath(prjpath,dir){
        var commudir = fileos.getPath(prjpath+"/"+dir+"/common");
        var windir = fileos.getPath(prjpath+"/"+dir+"/win32");
        var linuxdir = fileos.getPath(prjpath+"/"+dir+"/x86");

        return this._fileIsDirectory(commudir) || this._fileIsDirectory(windir) || this._fileIsDirectory(linuxdir);
    }
    _copyInstallPath(prjpath,prj,outputpath){        
		if(!prj.installdir || prj.installdir == "") prj.installdir = "install";
        if(fs.existsSync(fileos.getPath(prjpath+"/"+prj.installdir))){
			if(this._isInstallCommuPath(prjpath,prj.installdir)){
				if(fs.existsSync(fileos.getPath(prjpath+"/"+prj.installdir+"/common"))){
					fileos.copyDir(fileos.getPath(prjpath+"/"+prj.installdir+"/common/*"),outputpath);
				}
				if(fs.existsSync(fileos.getPath(prjpath+"/"+prj.installdir+"/"+(os.platform() == "win32"?"win32":"x86")))){
					fileos.copy(fileos.getPath(prjpath+"/"+prj.installdir+"/"+(os.platform() == "win32"?"win32":"x86")+"/*"),outputpath);
				}
			}else{
				fileos.copyDir(fileos.getPath(prjpath+"/"+prj.installdir+"/*"),outputpath);
			}
		}   
    } 
    _findNugetPath(currpath,nugetname){
        if(!fs.existsSync(fileos.getPath(`${currpath}/packages`))){
            return "";
        }
        var files = fs.readdirSync(fileos.getPath(`${currpath}/packages`));
        for (var i = 0; i < files.length; i++) {
            var verstartpos = files[i].indexOf(".");
            if(verstartpos == -1) continue;
            var name = files[i].substring(0,verstartpos);
            if(name == nugetname) return `packages/${files[i]}`
        }
        return "";
    }
    _replaseNugetLibPath(currpath,path){
        while(1){
            var startflag = "{nugetlib}";
            var stopflag = "{/nugetlib}";
            var nugetlibpos = path.indexOf(startflag);
            if(nugetlibpos == -1) break;
            var nugettmp = path.substring(nugetlibpos+startflag.length);
            var nugetlibstoppos = nugettmp.indexOf(stopflag);
            if(nugetlibstoppos == -1) break;
            var nugetname = nugettmp.substring(0,nugetlibstoppos);

            path = path.substring(0,nugetlibpos)+this._findNugetPath(currpath,nugetname) +"/build/native/lib/"+os.platform()+nugettmp.substring(nugetlibstoppos+stopflag.length);
        }
        return path;
    }
    _getDynamicDependItem(currpath,debug,path){
        path = path.replace("{_debug}",debug?"_debug":"");
        path = this._replaseNugetLibPath(currpath,path);

        return path;
    }
    _copyDynamicDepend(currpath,prjpath,prj,outputpath,debug){
        if(!prj.dynamicDepend || prj.dynamicDepend.length <= 0) return;

        for(var i = 0;i < prj.dynamicDepend.length;i ++){
            var dfile = this._getDynamicDependItem(currpath,debug,prj.dynamicDepend[i]); 
            var prjfile = prjpath + "/" + dfile;
            var pktfile = currpath == prjpath ? "" : (currpath + "/" + dfile);
            this._copyFileToPath(currpath,prjfile,outputpath);
            this._copyFileToPath(currpath,pktfile,outputpath);
        }
    }
    _findDCAPPFile(currpath,debug){
       var dcappfilename = "";

        if(fs.existsSync(fileos.getPath(currpath + "/packages"))){
            var dcappname = "DC_App";
		  var dcapppath = "";
			var files = fs.readdirSync(fileos.getPath(currpath + "/packages"));
			for (var i = 0; i < files.length; i++) {
				if(files[i].length < dcappname.length) continue;
				var filename = files[i].substring(0,dcappname.length);
				if(filename.toLocaleLowerCase() == dcappname.toLocaleLowerCase()){
					dcapppath = fileos.getPath(currpath + "/packages/"+files[i]+"/build/native/Bin/"+os.platform()+"/");
					break;
				}
			}
			if(dcapppath != "") dcappfilename = fileos.getPath(dcapppath+"DC_App"+(debug?"_debug":"")+((os.platform() == "win32"?".exe":"")));
        }
        else if(fs.existsSync(fileos.getPath(currpath+"/Bin/"+os.platform()+"/"+"DC_App"+(debug?"_debug":"")+(os.platform() == "win32"?".exe":""))))
		{
			dcappfilename = fileos.getPath(currpath+"/Bin/"+os.platform()+"/"+"DC_App"+(debug?"_debug":"")+(os.platform() == "win32"?".exe":""));
		}
        return dcappfilename;
    }
    _copyExenameFile(currpath,prjpath,prj,outputpath,debug){
        if(!prj.exename || prj.exename == "") return;

        var exefilename = fileos.getPath(prjpath + "/Bin/"+(os.platform() == "win32"?"win32":"x86")+"/"+prj.exename+(debug?"_debug":"")+(os.platform() == "win32"?".exe":""));
	    var binexefilename = fileos.getPath(currpath + "/Bin/"+(os.platform() == "win32"?"win32":"x86")+"/"+prj.exename+(debug?"_debug":"")+(os.platform() == "win32"?".exe":""));
        var dllfilename = fileos.getPath(prjpath + "/Lib/"+(os.platform() == "win32"?"win32":"x86")+"/"+prj.exename+"/"+prj.exename+(debug?"_debug":"")+(os.platform() == "win32"?".dll":".so"));
        var dllfilename2 = fileos.getPath(currpath + "/Lib/"+(os.platform() == "win32"?"win32":"x86")+"/"+prj.exename+"/"+prj.exename+(debug?"_debug":"")+(os.platform() == "win32"?".dll":".so"));

        var dllexefilename = fileos.getPath(prjpath + "/Lib/"+prj.exename+"/"+(os.platform() == "win32"?"win32":"x86")+"/"+prj.exename+(debug?"_debug":"")+(os.platform() == "win32"?".dll":".so"));
        var dllexefilename2 = fileos.getPath(currpath + "/Lib/"+prj.exename+"/"+(os.platform() == "win32"?"win32":"x86")+"/"+prj.exename+(debug?"_debug":"")+(os.platform() == "win32"?".dll":".so"));

        this._copyFileToPath(currpath,exefilename,outputpath);
        if(currpath != prjpath)
	        this._copyFileToPath(currpath,binexefilename,outputpath);
        this._copyFileToPath(currpath,dllfilename,outputpath);
        if(currpath != prjpath)
            this._copyFileToPath(currpath,dllfilename2,outputpath);

        this._copyFileToPath(currpath,dllexefilename,outputpath);
        if(currpath != prjpath)
            this._copyFileToPath(currpath,dllexefilename2,outputpath);

        var dcappname = this._findDCAPPFile(currpath,debug);
        if((fs.existsSync(fileos.getPath(dllfilename))||fs.existsSync(fileos.getPath(dllexefilename)) )&& dcappname != "" && fs.existsSync(dcappname)){
            this._copyFileToPath(currpath,dcappname,outputpath);

            if(fs.existsSync(fileos.getPath(outputpath+"/DC_App"+(debug?"_debug":"")+(os.platform() == "win32"?".exe":"")))){
                fs.renameSync(fileos.getPath(outputpath+"/DC_App"+(debug?"_debug":"")+(os.platform() == "win32"?".exe":"")), fileos.getPath(outputpath+"/"+prj.exename+(os.platform() == "win32"?".exe":"")));
                if(fs.existsSync(fileos.getPath(outputpath+"/"+prj.exename+(debug?"_debug":"")+(os.platform() == "win32"?".dll":".so")))){
                    fs.renameSync(fileos.getPath(outputpath+"/"+prj.exename+(debug?"_debug":"")+(os.platform() == "win32"?".dll":".so")), fileos.getPath(outputpath+"/"+prj.exename+(os.platform() == "win32"?".dll":".so")));
                }
            }
        }

		//debug的需要重新命名
		if(debug && fs.existsSync(fileos.getPath(outputpath+"/"+prj.exename+"_debug"+(os.platform() == "win32"?".exe":"")))){
			fs.renameSync(fileos.getPath(outputpath+"/"+prj.exename+"_debug"+(os.platform() == "win32"?".exe":"")), fileos.getPath(outputpath+"/"+prj.exename+(os.platform() == "win32"?".exe":"")));
		}	
     }	
    _fileIsDirectory(file){
        if(!fs.existsSync(file)) return false;

        var stat = fs.statSync(file);
        return stat.isDirectory();
    }
    _copyFileToPath(currpath,cpfile,topath){
        if(!cpfile || cpfile == "") return;

        var file = cpfile;
        var isDirctory = false;
        var copyAllfile = false;
        if(file.charAt(file.length - 1) == "*"){
            isDirctory = true;
            copyAllfile = true;
            file = file.substr(0,file.length - 1);
        }

        if(!fs.existsSync(fileos.getPath(file))) return;

        if(isDirctory || this._fileIsDirectory(file)){
            if(!copyAllfile){
                fileos.copyDir(file,topath);
            }else{
                fileos.copy(cpfile,topath);
            }
            
        }else{
            var filetype = file.lastIndexOf(".") == -1 ? "":file.substring(file.lastIndexOf(".") + 1,file.length);
            if(os.platform() == "win32"){ 
                if(filetype.toLocaleLowerCase() == "exe" || filetype.toLocaleLowerCase() == "dll"){
                    var cmd = "call \""+fileos.getPath(currpath + "\\mk\\pkt\\tool\\c_dll.exe")+"\" --dll \"" + file + "\" --targetdir \"" +fileos.getPath(topath) + "\" --dlldir \"" + fileos.getPath(currpath)+ "\" --pdb 1";
                    cp.execSync(cmd);
                }else{
                    fileos.copy(file,topath);
                }
            }else{
                new CopyDll().copy(currpath,file,topath);
            }
        }
    }
    _copydependProject(outputpath,prj,currpath,debug){
        if(!prj.dependProject || prj.dependProject.length <= 0) return;

        for(var i = 0;i < prj.dependProject.length;i ++){
            var dprj = this._getproject(currpath,prj.dependProject[i]);
            if(!dprj || dprj == null) continue;
            var doutput = this._package(currpath,prj.dependProject[i],debug);
            if(!doutput) continue;

            if(!fs.existsSync(doutput)) continue;
            this._checkPrjOutputType(dprj);

            if(dprj.type.toLocaleLowerCase() == "zip"){
                oszip.unzip(doutput,outputpath);
            }else if(dprj.type.toLocaleLowerCase() == "exe"){
                fileos.copy(doutput,outputpath);
            }
            fileos.rmfile(doutput);
        }
    } 
    _createDir(dir){
        if(fs.existsSync(dir)){
            fileos.rmdirforce(dir);
        }
        fs.mkdirSync(dir,777);
    }
    _createOutputDir(prjpath,prj){
        var dir = this._getPrjectOutputName(prjpath,prj);
        this._createDir(dir);

        return dir;
    }
    _getPrjectOutputName(prjpath,prj){
        var dir = fileos.getPath(prjpath + "/__"+prj.name+"_tmp");
        return dir;
    }
    _checkPrjWorkPath(currpath,prj){
        if(prj.dir && !fs.existsSync(fileos.getPath(currpath + "/"+prj.dir))){
            log.log("Project ["+name+"] WorkDIR ["+prj.dir+"] Not Exists");
            return false;
        }
        return true;
    }
    _checkPrjOutputType(prj){
        if(!prj.type) prj.type = "zip";

        prj.type = prj.type.toLocaleLowerCase();

        return prj.type == "zip" || prj.type == "exe";
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
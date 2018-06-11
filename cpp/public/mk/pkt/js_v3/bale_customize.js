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

module.exports = class Bale_Customize {
    about(){
        log.printdebuginfo(2,"default/[ProjectName] [debug/d]","打包用户自定义项目工程");
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
        log.log("Start Bale Customize ["+projectname+"]");
        this._package(currpath,projectname,debug);
        log.log("Success Bale Customize ["+projectname+"]");
    }

    _package(currpath,name,debug){
        //获取项目配合
        var prj = this._getproject(currpath,name);
        if(prj == null){
            log.log("Not Found Customize ["+name+"] Project Info!");
            return;
        }
        if(!prj.type || prj.type.toLocaleLowerCase() != "customize"){
            log.log("Customize ["+name+"] Not Support The Bale Type");
            return;
        }

        //创建输出目录
        var outputpath = this._createOutputDir(currpath,prj);

        this._packagecustomize(currpath,name,outputpath,debug);

        return this._zipoutputpath(outputpath,currpath,name,debug);
    }
    _zipoutputpath(outputpath,todir,name,debug,zipname){
        var outputfile = zipname ? zipname : (fileos.getPath(todir + "/" + name) + ".zip");
         if(fs.existsSync(fileos.getPath(outputfile))){
             fileos.rmfile(fileos.getPath(outputfile));
         }
        
        if(fs.existsSync(outputfile)){
            fileos.rmfile(outputfile);
        }
        this._backuppdb(outputpath,debug);
        this._removeDelete(outputpath,debug);
        oszip.zip(outputfile,outputpath+"\\");
    
        if(fs.existsSync(outputpath)){
            fileos.rmdirforce(outputpath);
        }

        if(!fs.existsSync(fileos.getPath(outputfile))){
            log.log("Bale Error,Output File ["+outputfile+"] Not Exists");
            return;
        }

        return fileos.getPath(outputfile);
    }
    _packagecustomize(currpath,name,outputpath,debug){
        var custiomizejson = fileos.getPath(currpath + "/customize.json");
        if (!fs.existsSync(custiomizejson))
        {
            log.log("Customize ["+name+"] Not Found customize.json");
            return;
        }
        var cfg = JSON.parse(fs.readFileSync(custiomizejson));
        
        if(!cfg.customize)
        {
            log.log("Customize ["+name+"] customize.json Filed 'customize' Error");
            return;
        }

        for(var i = 0;i < cfg.customize.length;i ++){
            var outputname = (cfg.customize[i].alias && cfg.customize[i].alias != "")?cfg.customize[i].alias:cfg.customize[i].name;
            var myprjpath = currpath + "\\" + cfg.customize[i].path;
            var myoutputpath = this._createOutputDir(currpath +"\\"+cfg.customizeOutput,cfg.customize[i]);

            //拷贝输出的exe文件
            this._copyExenameFile(currpath,cfg.customizeOutput,cfg.customize[i].name,myoutputpath,debug);
            //拷贝动态依赖文件
            this._copyDynamicDepend(currpath,myprjpath,cfg.customize[i],myoutputpath,debug);
            //拷贝安装目录文件
            this._copyInstallPath(myprjpath,cfg.customize[i],myoutputpath);
            //更新安装文件夹
            this._replaceInstallFile(myprjpath,cfg.customize[i],myoutputpath,debug);
            
            var version = new Version();
            var ver = version.versionString(false);

            var date = new Date();

            var zipname = outputpath+"\\"+outputname+"_"+os.platform()+"_"+ (debug ? "D" : "R") + "_" +ver+"_"+fileos.date("yyyyMMdd")+".zip";
            //压缩
            this._zipoutputpath(myoutputpath,outputpath,outputname,debug,zipname);
        }
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
    _replaceInstallFile(prjpath,prj,tmpdirname,debug){
		
		var exename = os.platform() == "win32" ? prj.name+".exe" : prj.name;

        var version = new Version();
        var ver = version.versionString(false);

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
    _copyExenameFile(currpath,libpath,libname,outputpath,debug){
        var dllfilename = fileos.getPath(currpath +"\\"+libpath +"\\"+libname+"\\"+(os.platform() == "win32"?"win32":"x86")+"\\"+libname+(debug?"_debug":"")+(os.platform() == "win32"?".dll":".so"));
        
        this._copyFileToPath(currpath,dllfilename,outputpath);
    }
    _copyDynamicDepend(currpath,prjpath,prj,outputpath,debug){
        if(!prj.pktdepend || prj.pktdepend.length <= 0) return;

        for(var i = 0;i < prj.pktdepend.length;i ++){
            if(!prj.pktdepend[i] || prj.pktdepend == "") continue;
            var dfile = prj.pktdepend[i].replace("{_debug}",debug?"_debug":"");
            var prjfile = prjpath + "/" + dfile;
            this._copyFileToPath(currpath,prjfile,outputpath);
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
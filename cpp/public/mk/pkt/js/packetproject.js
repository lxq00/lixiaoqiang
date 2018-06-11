const fs = require("fs");
const os = require("os");
const cp = require('child_process');

const log = require('./pktlog.js');
const fileos = require('./fileos.js');
const oszip = require("./oszip.js");
const update = require("./updatedepend.js");
const ms = require("./mkshared.js");

let packetproject = module.exports;

var currpath = ms.currpath();


packetproject.pktCustomize = function (currpath,ver,customizeOutput,customize,debug,full)
{
    var outputdir = fileos.getPath(currpath+"\\"+(customizeOutput ? customizeOutput : "Lib")+"\\"+(customize.alias == "" ? customize.name:customize.alias)+"\\"+(os.platform()?"win32":"x86"));
    var outputfile = fileos.getPath(outputdir+"\\"+customize.name+(debug?"_debug":"")+".dll");
    if (!fs.existsSync(outputfile))
    {
        return "";
    }

    var tmpdirname = outputdir + "\\_pack_tmp"+(debug?"_debug":"")+(full?"_full":"");
    if (fs.existsSync(tmpdirname))
	{
		fileos.rmdir(tmpdirname);
	}
	
    fs.mkdirSync(tmpdirname,0777);
    copydepend(tmpdirname+"\\",outputfile);

    for(var i = 0;i < customize.pktdepend.length;i ++)
    {
        if(customize.pktdepend[i] == "" || customize.path == "")
        {
            continue;
        }
        var dependfile = currpath + "\\" + customize.path + "\\" + customize.pktdepend[i] ;
        if (!fs.existsSync(dependfile))
        {
            continue;
        }
        fileos.copyDir(dependfile+"\\*",tmpdirname);
    }

    removeDelete(tmpdirname,debug || full,false);
    reWritecfg(tmpdirname,ver,"",customize.readme);

    var date = new Date();

    var zipname = fileos.getPath(outputdir + "/" + (customize.alias == "" ? customize.name:customize.alias) + "_" + os.platform() + (debug ? "_D" : "_R") + "_" + ver+"_"+fileos.date("yyyyMMdd") + ((!debug&&full) ? "_full":"")+".zip");

    oszip.zip(zipname,tmpdirname+ "\\");

    fileos.rmdir(tmpdirname);

    return zipname;
}

packetproject.packLibrary = function(cfg,prjpath,fversion,buildcfg)
{
    if (cfg.ProjectName.toLocaleLowerCase() == "vgsii_public" && !fs.existsSync(prjpath + "/_Output")) {
        return;
    }
    var ver = fversion+"_"+fileos.date("yyyyMMdd");

    if(cfg.ProjectName.toLocaleLowerCase() == "vgsii_public")
    {
        update.updateOutput(ver,cfg,buildcfg);
    }
    else
    {
        if(cfg.ProjectName.toLocaleLowerCase() == "vgsii_dcapp")
        {
            update.updateBin(prjpath,ver,"dcapp");
        }        
        update.updateLib(prjpath,ver,cfg);
    }
}

packetproject.packApp = function(cfg,prjpath,ver,debug)
{
    var pktlist = [];
    var zipdebug = debug ? packZip(cfg, prjpath,ver, true,true) : "";
    var ziprealse = packZip(cfg, prjpath,ver, false,false);	
    var ziprealsefull = "";
    var otherzip = "";
    if (os.platform() == "win32")
    {
        ziprealsefull = packZip(cfg, prjpath,ver, false,true);
        otherzip = buildOtherZip(cfg.OtherOutput,ver,prjpath,ziprealse);
    }
    if(otherzip != "") pktlist.push(otherzip);
    if(zipdebug != "") pktlist.push(zipdebug);
    if(ziprealse != "") pktlist.push(ziprealse);
    if(ziprealsefull != "") pktlist.push(ziprealsefull);
    
    return pktlist;
}
function buildOtherZip(OtherOutput,ver,prjpath,releasefile)
{
    if(!OtherOutput || !OtherOutput.name || !OtherOutput.cmd){return "";}

    var date = new Date();
    var zipname = prjpath + OtherOutput.name.replace("{Platform}", os.platform() == "win32" ? "win32" : (os.platform() == "linux" ? "Linux":""));
    zipname = zipname.replace("{PublishTime}", date.format("yyyyMMdd"));
    zipname = zipname.replace("{version}",ver);

    cp.execSync("call \""+OtherOutput.cmd+"\" \""+currpath+"\" \""+prjpath+"\" "+ver + " \""+releasefile+"\" \""+zipname+"\"");   
  
    return zipname;
}
function packZip(cfg,prjpath,ver,debug,havepdb=true)
{
    var tmpdirname = fileos.getPath(prjpath + "/_pack_tmp" + (debug ? "_debug" : "") + (havepdb?"_pdb":"")+"/");
	if (fs.existsSync(tmpdirname))
	{
		fileos.rmdir(tmpdirname);
	}
	
    fs.mkdirSync(tmpdirname,0777);

    if (fs.existsSync(fileos.getPath(prjpath + "/install/common"))) {
        fileos.copyDir(fileos.getPath(prjpath + "/install/common/*"), tmpdirname);
    }
    if (os.platform() == "win32" && fs.existsSync(fileos.getPath(prjpath + "/install/win32")))
	{
		fileos.copyDir(fileos.getPath(prjpath + "/install/win32/*"), tmpdirname);
	}
	else if (os.platform() == "linux" && fs.existsSync(fileos.getPath(prjpath + "/install/x86")))
	{
		fileos.copyDir(fileos.getPath(prjpath + "/install/x86/*"), tmpdirname);
	}

    //�ȴ������������Ƶ����ؿ���Ӧ�ó���
    if (os.platform() == "win32" && fs.existsSync(fileos.getPath(prjpath + "/Lib/"+cfg.ProjectOutput+"/win32/"))) {
        copydepend(tmpdirname,prjpath + "\\Lib\\"+cfg.ProjectOutput+"\\win32\\" + cfg.ProjectOutput + (debug ? "_debug" : "") + ".dll");
    }
    if (os.platform() == "win32")
	{
		if(fs.existsSync(fileos.getPath(prjpath + "/Bin/win32/" + cfg.ProjectOutput +(debug ? "_debug" : "") + ".exe"))) {
            copydepend(tmpdirname,prjpath + "\\Bin\\win32\\" + cfg.ProjectOutput + (debug ? "_debug" : "") + ".exe");
			if (debug) {
				fs.renameSync(tmpdirname + cfg.ProjectOutput + "_debug.exe", tmpdirname + cfg.ProjectOutput + ".exe")
			}
		}
		else if(fs.existsSync(fileos.getPath(currpath + "/Bin/win32/DC_App"+(debug ? "_debug" : "") + ".exe"))) {
            copydepend(tmpdirname,currpath + "\\Bin\\win32\\DC_App"+(debug ? "_debug" : "") +".exe");
			if (debug) {
				fs.renameSync(tmpdirname + "\\"+cfg.ProjectOutput + "_debug.dll", tmpdirname + cfg.ProjectOutput + ".dll")
			}
			fs.renameSync(tmpdirname + "DC_App"+(debug ? "_debug" : "") + ".exe", tmpdirname + cfg.ProjectOutput + ".exe")
		}
    }
    else if (os.platform() == "linux")
	{
		if(fs.existsSync(prjpath + "/Bin/x86/" + cfg.ProjectOutput + (debug ? "_debug" : ""))) {
			fileos.copy(prjpath + "/Bin/x86/" + cfg.ProjectOutput + (debug ? "_debug" : ""), tmpdirname + cfg.ProjectOutput + (debug ? "_debug" : ""));
			if (debug) {
				fs.renameSync(tmpdirname + cfg.ProjectOutput + "_debug", tmpdirname + cfg.ProjectOutput)
			}
		}
		else if(fs.existsSync(currpath + "/Bin/win32/DC_App"+(debug ? "_debug" : ""))) {
			fileos.copy(currpath + "/Bin/win32/DC_App"+ (debug ? "_debug" : ""), tmpdirname);
			fs.renameSync(tmpdirname + "DC_App"+(debug ? "_debug" : ""), tmpdirname + cfg.ProjectOutput)
		}
    }
    //����������
    if (cfg.Package.App.Depends) {
        for(var i = 0;i < cfg.Package.App.Depends.length;i++)
        {
            if (os.platform() == "win32" && fs.existsSync(fileos.getPath(currpath + "/_Output/Lib/win32/" + cfg.Package.App.Depends[i])))
            {
                copydepend(tmpdirname,currpath + "\\_Output\\Lib\\win32\\" + cfg.Package.App.Depends[i] + "\\"+cfg.Package.App.Depends[i] + (debug ? "_debug" : "") + ".dll");
            }
            if (os.platform() == "win32" && fs.existsSync(fileos.getPath(prjpath + "/Lib/win32/" + cfg.Package.App.Depends[i]))) {
                copydepend(tmpdirname,prjpath + "\\Lib\\win32\\"+cfg.Package.App.Depends[i]+"/" + cfg.Package.App.Depends[i] + (debug ? "_debug" : "") + ".dll");
            }
        }
    }
    
    //ִ����Ҫ��������Ϣ
    if (os.platform() == "win32" && fs.existsSync(fileos.getPath(prjpath + "/packIng.bat"))) {
        cp.execSync(fileos.getPath("call \""+prjpath + "/packIng.bat\" "+(debug?"debug":"release") + " \""+tmpdirname+"\" \"" + prjpath + "\" "+ver + " "+(havepdb ? "full":"nofull")));
    }
    else if (os.platform() == "linux" && fs.existsSync(prjpath + "/packIng.sh")) {
        cp.execSync("/bin/sh \""+prjpath + "/packIng.sh\" "+ (debug ?"debug" : "release")+" \""+tmpdirname+"\" \"" + prjpath + "\" "+ver + " "+(havepdb ? "full":"nofull"));
    }

    removeDelete(tmpdirname,debug || havepdb,true);
    reWritecfg(tmpdirname,ver,cfg.ProjectOutput + (os.platform() == "win32"?".exe":""));

    if (os.platform() == "linux")
    {
		cp.execSync("chmod  -R 777 \""+tmpdirname+"\"");
    }
    var date = new Date();

    var zipname = fileos.getPath(prjpath + "/" + cfg.ProjectName + "_" + os.platform() + (debug ? "_D" : "_R") + "_" + ver+"_"+fileos.date("yyyyMMdd") + ((!debug&&havepdb) ? "_full":"")+".zip");

    oszip.zip(zipname,tmpdirname);

    fileos.rmdir(tmpdirname);

    return zipname;
}

function copydepend(topath,dll)
{
    cp.execSync("\""+fileos.getPath(currpath + "\\mk\\pkt\\tool\\c_dll.exe")+"\" --dll \"" + dll + "\" --targetdir \"" +fileos.getPath(topath) + "\\\" --dlldir \"" + fileos.getPath(currpath)+ "\\\\\" --pdb 1");
}

function  fileIsDirectory(file){
	if(!fs.existsSync(file)) return false;

	var stat = fs.statSync(file);
	return stat.isDirectory();
}

function removeDelete(tmpdirname,havepdb,haveexe)
{
	var files = fs.readdirSync(fileos.getPath(tmpdirname));
	for(var i = 0;i < files.length;i ++){
		if(fileIsDirectory(tmpdirname+"//"+files[i])){
			removeDelete(tmpdirname+"//"+files[i],havepdb,haveexe);
		}
	}

    if(!havepdb)
    {
        fileos.rmfile(tmpdirname+"\\*.pdb",false);
    }
    if(!haveexe)
    {
        fileos.rmfile(tmpdirname+"\\*.exe",false);
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
}

function reWritecfg(tmpdirname,ver,exename,customizereadme)
{
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
        xmldata = xmldata.replace("{customizereadme}", customizereadme ? customizereadme : "");
        fs.writeFileSync(fileos.getPath(tmpdirname + "/readme.md"), xmldata);
    }
}

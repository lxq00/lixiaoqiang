const fs = require("fs");
const os = require("os");
const cp = require('child_process');
const pr = require('process');
const path = require('path');
const fileos = require('./js/fileos.js');
const Version = require("./js_v3/version.js");
const Event = require("./js_v3/event.js")
const oszip = require("./js/oszip.js");
const Sln = require('./js_v4/sln.js');
const log = require("./js/pktlog.js")


//{files} <file src="..\..\include\*.h" target="build\native\include" />
//{dependency} <dependency id="name" version="111" />
function _repleaseFile(info, filename) {
    var evendata = fs.readFileSync(fileos.getPath(filename));

    while (evendata.indexOf("{name}") > -1) evendata = evendata.toString().replace("{name}", info.name);
    while (evendata.indexOf("{version}") > -1) evendata = evendata.toString().replace("{version}", info.version);
    while (evendata.indexOf("{authors}") > -1) evendata = evendata.toString().replace("{authors}", info.authors);
    while (evendata.indexOf("{giturl}") > -1) evendata = evendata.toString().replace("{giturl}", info.giturl);
    while (evendata.indexOf("{summary}") > -1) evendata = evendata.toString().replace("{summary}", info.summary);
    while (evendata.indexOf("{description}") > -1) evendata = evendata.toString().replace("{description}", info.description);
    while (evendata.indexOf("{files}") > -1) evendata = evendata.toString().replace("{files}", info.files);
    //   while(evendata.indexOf("{tags}") > -1)  evendata = evendata.toString().replace("{tags}",info.tags);
    while (evendata.indexOf("{dependency}") > -1) evendata = evendata.toString().replace("{dependency}", info.dependency);
    while (evendata.indexOf("{definitions}") > -1) evendata = evendata.toString().replace("{definitions}", info.definitions);
    //while (evendata.indexOf("{include}") > -1) evendata = evendata.toString().replace("{include}", info.include);
    while (evendata.indexOf("{include_x86}") > -1) evendata = evendata.toString().replace("{include_x86}", info.include_x86);
    while (evendata.indexOf("{include_win32}") > -1) evendata = evendata.toString().replace("{include_win32}", info.include_win32);
    while (evendata.indexOf("{lib_debug_x86}") > -1) evendata = evendata.toString().replace("{lib_debug_x86}", info.lib_debug_x86);
    while (evendata.indexOf("{lib_release_x86}") > -1) evendata = evendata.toString().replace("{lib_release_x86}", info.lib_release_x86);
    while (evendata.indexOf("{lib_debug_win32}") > -1) evendata = evendata.toString().replace("{lib_debug_win32}", info.lib_debug_win32);
    while (evendata.indexOf("{lib_release_win32}") > -1) evendata = evendata.toString().replace("{lib_release_win32}", info.lib_release_win32);

    fs.writeFileSync(fileos.getPath(filename), evendata);
}

function _builddepend(depend, currpath) {
    if (!depend) return "";
    var dependstr = "";
    while (true) {
        var dependendpos = depend.indexOf(";");
        if (dependendpos == -1) break;
        var datatmp = depend.toString().substring(0, dependendpos);

        var verstartpos = datatmp.indexOf("=");
        if (verstartpos == -1) break;

        var name = datatmp.substring(0, verstartpos);
        var version = datatmp.substring(verstartpos + 1);
        if (version == "") {
            version = new Version().versionString(false, name, currpath)
        }
        if (version != "") {
            var verarray = new Version()._buildFullVersion(version);
            version = `[${version},${verarray[0] + 1}.0.0)`;
        }

        dependstr += `<dependency id=\"${name}\" version=\"${version}\" />\r\n`;

        depend = depend.toString().substring(dependendpos + 1);
    }

    return dependstr;
}

function _getgiturl(currpath) {
    try {
        var gitpath = new Event().event().win32.git;
        var cmd = "cd \"" + currpath + "\" && call \"" + gitpath + "/bin/git.exe\" remote -v";
        var filetmp = cp.execSync(fileos.getPath(cmd)).toString();
        var firstpos = filetmp.indexOf("http://");
        if (firstpos == -1) return filetmp;
        filetmp = filetmp.substring(firstpos);
        var endpost = filetmp.indexOf(" ");
        if (endpost == -1) return filetmp;
        filetmp = filetmp.substring(0, endpost);

        var namepos = filetmp.indexOf("@");
        if (namepos == -1) return filetmp;

        return "http://" + filetmp.substring(namepos + 1);
    } catch (error) {
        return "http://127.0.0.1";
    }
}
function _getObjectInfo(currpath, linuxpath, name, version,includerootpath) {
    var objinfo = {};

    objinfo.name = name;
    objinfo.version = version;

    {
        objinfo.include = objinfo.include_x86 = objinfo.lib_debug_x86 = objinfo.lib_release_x86 = objinfo.lib_debug_win32 = objinfo.lib_release_win32 = "";

        if (fs.existsSync(fileos.getPath(`${currpath}/Include/${name}`))) {
            objinfo.include_win32 = `$(MSBuildThisFileDirectory)../..//build/native/include/;`+(includerootpath?`$(MSBuildThisFileDirectory)../..//build/native/include/${name}/;`:``);
            objinfo.include_x86 = `-I {NugetPath}/build/native/include/`+(includerootpath?` -I {NugetPath}/build/native/include/${name}/`:``);
        }else if(fs.existsSync(fileos.getPath(`${currpath}/Include/`))) {
            if(fs.existsSync(fileos.getPath(`${currpath}/Include/win32`)) || fs.existsSync(fileos.getPath(`${currpath}/Include/x86`))){
                if(fs.existsSync(fileos.getPath(`${currpath}/Include/win32`))) {
                    objinfo.include_win32 = `$(MSBuildThisFileDirectory)../..//build/native/include/win32;`;
                }
                if(fs.existsSync(fileos.getPath(`${currpath}/Include/x86`))) {
                    objinfo.include_x86 = `-I {NugetPath}/build/native/include/x86`;
                }
            }else{
                objinfo.include_win32 = `$(MSBuildThisFileDirectory)../..//build/native/include;`;
                objinfo.include_x86 = `-I {NugetPath}/build/native/include`;
            }            
        }

        if (fs.existsSync(fileos.getPath(`${currpath}/Lib/win32/${name}/${name}.lib`))) {
            objinfo.lib_release_win32 = `${name}.lib`;
        }
        if (fs.existsSync(fileos.getPath(`${currpath}/Lib/win32/${name}/${name}_debug.lib`))) {
            objinfo.lib_debug_win32 = `${name}_debug.lib`;
        }
        if (linuxpath && fs.existsSync(fileos.getPath(`${linuxpath}/Lib/x86/${name}`))) {
            if (fs.existsSync(fileos.getPath(`${linuxpath}/Lib/x86/${name}/lib${name}_debug.a`)) ||
                fs.existsSync(fileos.getPath(`${linuxpath}/Lib/x86/${name}/lib${name}_debug.so`))) {
                objinfo.lib_debug_x86 = `-L {NugetPath}/build/native/lib/x86/ -Wl,-rpath={NugetPath}/build/native/lib/x86/ -l${name}_debug`;
            }
            if (fs.existsSync(fileos.getPath(`${linuxpath}/Lib/x86/${name}/lib${name}.a`)) ||
                fs.existsSync(fileos.getPath(`${linuxpath}/Lib/x86/${name}/lib${name}.so`))) {
                objinfo.lib_release_x86 = `-L {NugetPath}/build/native/lib/x86/ -Wl,-rpath={NugetPath}/build/native/lib/x86/ -l${name}`;
            }
        }else if (linuxpath && fs.existsSync(fileos.getPath(`${linuxpath}/Lib/x86/`))){
            objinfo.lib_debug_x86 = `-L {NugetPath}/build/native/lib/x86/ -Wl,-rpath={NugetPath}/build/native/lib/x86/`;
            objinfo.lib_release_x86 = `-L {NugetPath}/build/native/lib/x86/ -Wl,-rpath={NugetPath}/build/native/lib/x86/`;
        }
    }
    {
        objinfo.summary = objinfo.tags = objinfo.dependency = objinfo.definitions = "";
        var versioninfo = {};
        if (fs.existsSync(fileos.getPath(currpath + "/Include/" + name + "/Version.tpl"))) {
            versioninfo = JSON.parse(fileos.readFileSync(fileos.getPath(currpath + "/Include/" + name + "/Version.tpl")));
        }
        else if (fs.existsSync(fileos.getPath(currpath + "/" + name + "/Version.tpl"))) {
            versioninfo = JSON.parse(fileos.readFileSync(fileos.getPath(currpath + "/" + name + "/Version.tpl")));
        }
        else if (fs.existsSync(fileos.getPath(currpath + "/Src/" + name + "/Version.tpl"))) {
            versioninfo = JSON.parse(fileos.readFileSync(fileos.getPath(currpath + "/Src/" + name + "/Version.tpl")));
        }
        else if (fs.existsSync(fileos.getPath(currpath + "/Version.tpl"))) {
            versioninfo = JSON.parse(fileos.readFileSync(fileos.getPath(currpath + "/Version.tpl")));
        }

        objinfo.summary = versioninfo.summary ? versioninfo.summary : objinfo.name;
        objinfo.tags = versioninfo.tags ? versioninfo.tags : "";
        objinfo.description = versioninfo.description ? versioninfo.description : objinfo.name;

        objinfo.dependency = _builddepend(versioninfo.dependency + _parseVcProjectAuto(currpath, name), currpath);
    }

    objinfo.authors = os.userInfo().username;
    objinfo.giturl = _getgiturl(currpath);

    return objinfo;
}

function _getNuspecFiles(includepath, win32libpath, linuxlibpath, binpath,name, pkttmpdir) {
    function _getIncludeFilesInPath(pathname,startpath,path) {

        var nuspecfiles = `<file src="${startpath}\\${path}*" target="build\\native\\${pathname}\\${path}" />\r\n`;

        var files = fs.readdirSync(`${startpath}\\${path}`);
        for (var i = 0; i < files.length; i++) {
            if (files[i].substring(0, 1) == "." || files[i] == "..") {
                continue;
            }
            var info = fs.statSync(fileos.getPath(`${startpath}\\${path}${files[i]}`));
            if (info.isDirectory()) {
                nuspecfiles += _getIncludeFilesInPath(pathname,startpath,path + files[i] + "\\")
            }
        }

        return nuspecfiles;
    }

    function _getBinFilesInPath(binpath){
        var nuspecfiles = "";
        for(var i = 0;i < binpath.length;i ++){
            nuspecfiles += _getIncludeFilesInPath(`${binpath[i].pathname}`,binpath[i].path,"");
        }

        return nuspecfiles;
    }


    var nuspecfiles = "";
    if (includepath != "" && path.parse(includepath).base == name) {
        nuspecfiles = _getIncludeFilesInPath(`include\\${name}`,includepath,"");
    }else if (includepath != "") {
        nuspecfiles = _getIncludeFilesInPath("include",includepath,"");
    }
    nuspecfiles += _getBinFilesInPath(binpath);
    if (win32libpath != "") {
        nuspecfiles += _getIncludeFilesInPath("lib\\win32",win32libpath,"");
    }
    if (linuxlibpath != "") {
        nuspecfiles += _getIncludeFilesInPath("lib\\x86",linuxlibpath,"");
    }

    nuspecfiles += `<file src="${pkttmpdir}/default-propertiesui.xml" target="build\\native" />\r\n`;
    nuspecfiles += `<file src="${pkttmpdir}/${name}.mk" target="build\\native" />\r\n`;
    nuspecfiles += `<file src="${pkttmpdir}/${name}.targets" target="build\\native" />\r\n`;

    return nuspecfiles;
}
function _buildNugetPacket(currpath,packgdir,linuxpath,nupkgoutputdir,pktobj,includepath,win32libpath,linuxlibpath,binpath){
    var name = pktobj.name;
    var version = new Version().versionString(false, name, packgdir);
    var pkttmpdir = fileos.getPath(`${nupkgoutputdir}/__${name}.${version}_nupkg`);

    if (fs.existsSync(pkttmpdir)) fileos.rmdir(pkttmpdir);
    fs.mkdirSync(pkttmpdir);

    fileos.copy(`${__dirname}/nugettmp/*`, pkttmpdir);

    var objinfo = _getObjectInfo(packgdir, linuxpath, name, version,pktobj.includrootpath);
    objinfo.files = _getNuspecFiles(includepath, win32libpath, linuxlibpath, binpath,name, pkttmpdir);

    _repleaseFile(objinfo, `${pkttmpdir}/{name}.mk`); fileos.move(`${pkttmpdir}/{name}.mk`, `${pkttmpdir}/${objinfo.name}.mk`)
    _repleaseFile(objinfo, `${pkttmpdir}/{name}.nuspec`); fileos.move(`${pkttmpdir}/{name}.nuspec`, `${pkttmpdir}/${objinfo.name}.nuspec`)
    _repleaseFile(objinfo, `${pkttmpdir}/{name}.targets`); fileos.move(`${pkttmpdir}/{name}.targets`, `${pkttmpdir}/${objinfo.name}.targets`)


    var cmd = `cd \"${pkttmpdir}\" && call ${__dirname}\\tool\\nuget.exe pack ${pkttmpdir}/${objinfo.name}.nuspec -verbosity detailed`;
    cp.execSync(fileos.getPath(cmd));

    var nuspecpath = "";
    if (fs.existsSync(`${pkttmpdir}/${objinfo.name}.${objinfo.version}.nupkg`)) {
        nuspecpath = `${pkttmpdir}/${objinfo.name}.${objinfo.version}.nupkg`;
    } else if (fs.existsSync(`${currpath}/${objinfo.name}.${objinfo.version}.nupkg`)) {
        nuspecpath = `${currpath}/${objinfo.name}.${objinfo.version}.nupkg`;
    }

    if (nuspecpath != "") {
        fileos.move(nuspecpath, `${nupkgoutputdir}/${objinfo.name}.${objinfo.version}.nupkg`)
        console.log(objinfo.name + "打包成功!");
    }
    else {
        console.log(objinfo.name + "打包失败!");
    }

    fileos.rmdir(pkttmpdir);
}
function _buildAllAutoPkt(currpath, packgdir, linuxpath, pktobj, nupkgoutputdir) {
    var name = pktobj.name;

    var includepath = "";
    var win32libpath = "";
    var linuxlibpath = "";
    var binpath = new Array();
    if (fs.existsSync(fileos.getPath(`${packgdir}/Include/${name}`))) includepath = fileos.getPath(`${packgdir}/Include/${name}`);
    if (fs.existsSync(fileos.getPath(`${packgdir}/Lib/win32/${name}`))) win32libpath = fileos.getPath(`${packgdir}/Lib/win32/${name}`);
    else if (fs.existsSync(fileos.getPath(`${packgdir}/Bin/win32/${name}.exe`))) binpath = binpath.concat({path:fileos.getPath(`${packgdir}/Bin/win32/`),pathname:"Bin\\win32"});
    if (linuxpath && fs.existsSync(fileos.getPath(`${linuxpath}/Lib/x86/${name}`))) linuxlibpath = fileos.getPath(`${linuxpath}/Lib/x86/${name}`);
    else if (linuxpath && fs.existsSync(fileos.getPath(`${linuxpath}/Bin/x86/${name}`))) binpath = binpath.concat({path:fileos.getPath(`${linuxpath}/Bin/x86/`),pathname:"Bin\\x86"});


    _buildNugetPacket(currpath,packgdir,linuxpath,nupkgoutputdir,pktobj,includepath,win32libpath,linuxlibpath,binpath);
}

function _packgeNuget(inputfile) {
    if (fs.existsSync(fileos.getPath(inputfile))) {
        var pktobj = JSON.parse(fileos.readFileSync(fileos.getPath(inputfile), currpath));

        var linuxpath = pktobj.linuxpath;
        var currpath = pktobj.currpath;
        var nupkgoutputdir = fileos.getPath(`${currpath}\\nupkg`);
        if (currpath && !fs.existsSync(nupkgoutputdir)) fs.mkdirSync(nupkgoutputdir);

        for (var i = 0; pktobj.result && i < pktobj.result.length; i++) {
            _buildAllAutoPkt(pr.cwd(), currpath, linuxpath, pktobj.result[i], nupkgoutputdir);
        }

        fileos.rmfile(inputfile);
    }
}

function _setpktinfo(inputfile) {
    if (fs.existsSync(fileos.getPath(inputfile))) {
        var pktobj = JSON.parse(fileos.readFileSync(fileos.getPath(inputfile), currpath));
        var linuxpath = pktobj.linuxpath;
        var currpath = pktobj.currpath;

        for (var i = 0; pktobj.result && i < pktobj.result.length; i++) {
            var prjpath = "";
            if (fs.existsSync(fileos.getPath(currpath + "/include/" + pktobj.result[i].name + "/Version.tpl"))) {
                prjpath = "/include/" + pktobj.result[i].name + "/Version.tpl";
            } else if (fs.existsSync(fileos.getPath(currpath + "/" + pktobj.result[i].name + "/Version.tpl"))) {
                prjpath = "/" + pktobj.result[i].name + "/Version.tpl";
            }
            if (prjpath == "") {
                continue;
            }

            var version = {};
            version.productVersion = pktobj.result[i].version;
            version.summary = pktobj.result[i].summary;
            version.definitions = pktobj.result[i].definitions;
            version.dependency = pktobj.result[i].dependency ? pktobj.result[i].dependency : "";
            version.description = pktobj.result[i].description;
            version.includrootpath = pktobj.result[i].includrootpath;
            fileos.writeFileSync(fileos.getPath(currpath + prjpath), JSON.stringify(version, null, 2));
            if (linuxpath && fs.existsSync(fileos.getPath(currpath + prjpath))) {
                fileos.writeFileSync(fileos.getPath(linuxpath + prjpath), JSON.stringify(version, null, 2));
            }
        }
        fileos.rmfile(inputfile);
    }
}

function _parseDependVCProject(currpath, prjname) {
    if (!fs.existsSync(fileos.getPath(`${currpath}/${prjname}/packages-depend.targets`))) {
        return "";
    }
    var filedata = fs.readFileSync(`${currpath}/${prjname}/packages-depend.targets`).toString();
    var startpos = filedata.indexOf(`<AdditionalDependencies>`);
    if (startpos == -1) return depend;
    filedata = filedata.substring(startpos);
    var endpos = filedata.indexOf(`</AdditionalDependencies>`);
    if (endpos == -1) return depend;

    filedata = filedata.substring(0, endpos);

    var depend = "";
    while (true) {
        var dataendpos = filedata.indexOf(";");
        var datatmp = filedata;
        if (dataendpos != -1) {
            datatmp = filedata.substring(0, dataendpos);
        }
        var libendtpos = datatmp.indexOf(".lib");
        if (libendtpos == -1) break;
        datatmp = datatmp.substring(0, libendtpos);
        var libstartpos = datatmp.lastIndexOf("/");
        if (libstartpos == -1) break;

        datatmp = datatmp.substring(libstartpos + 1, libendtpos);
        var debugstartpos = datatmp.indexOf("_debug");
        if (debugstartpos != -1) datatmp = datatmp.substring(0, debugstartpos);

        if (fs.existsSync(fileos.getPath(`${currpath}/Lib/win32/${datatmp}/${datatmp}.dll`))) {
            var config = null;
            if (fs.existsSync(fileos.getPath(currpath + "/Include/" + datatmp + "/Version.tpl"))) {
                config = JSON.parse(fileos.readFileSync(fileos.getPath(currpath + "/Include/" + datatmp + "/Version.tpl")));
            } else if (fs.existsSync(fileos.getPath(currpath + "/" + datatmp + "/Version.tpl"))) {
                config = JSON.parse(fileos.readFileSync(fileos.getPath(currpath + "/" + datatmp + "/Version.tpl")));
            }
            if (config != null && config.productVersion && config.productVersion != "") {
                depend += datatmp + "=" + config.productVersion + ";";
            }
        }


        filedata = filedata.substring(dataendpos + 1);
    }
    return depend;
}
function _parseVCProjectDepend(currpath, prjfile) {
    var filedata = fs.readFileSync(fileos.getPath(prjfile)).toString();
    var startflag = `ImportGroup Label="ExtensionTargets">`;
    var startpos = filedata.indexOf(startflag);
    if (startpos == -1) return "";
    filedata = filedata.substring(startpos + startflag.length);

    var endpos = filedata.indexOf("</ImportGroup>");
    if (endpos == -1) return "";

    filedata = filedata.substring(0, endpos);

    var dependstr = "";
    while (true) {
        var stoppos = filedata.indexOf("/>");
        if (stoppos == -1) break;
        var datatmp = filedata.substring(0, stoppos);

        var startflag = `\\packages\\`;
        var startpos = datatmp.indexOf(startflag);
        if (startpos == -1) break;
        datatmp = datatmp.substring(startpos + startflag.length);
        var dataendpos = datatmp.indexOf("\\build\\native\\");
        if (dataendpos == -1) break;

        datatmp = datatmp.substring(0, dataendpos);
        var verstart = datatmp.indexOf(".");
        if (verstart == -1) break;

        var name = datatmp.substring(0, verstart);
        var version = datatmp.substring(verstart + 1);

        if (fs.existsSync(fileos.getPath(`${currpath}/packages/${name}.${version}/build/native/lib/win32/${name}.dll`))) {
            dependstr += datatmp.substring(0, verstart) + "=" + version + ";";
        }

        filedata = filedata.substring(stoppos + 2);
    }

    return dependstr;
}
function _findvcprjfile(currpath,prjname){
    var files = fs.readdirSync(fileos.getPath(currpath));
    for (var i = 0; i < files.length; i++) {
        if (files[i].substring(0, 1) == "." || files[i] == "_Output" || files[i] == "__Compile" || files[i] == "Lib" || files[i] == "mk" || files[i] == "packages" || files[i] == "Compile") {
            continue;
        }
        var info = fs.statSync(fileos.getPath(currpath + "/" + files[i]));
        if (info.isDirectory()) {
            var vcprjname = _findvcprjfile(fileos.getPath(currpath + "/" + files[i]),prjname);
            if(vcprjname != "") {
                return vcprjname;
            }
        }
        if(path.parse(files[i]).ext.toLocaleLowerCase() != ".vcxproj"){
            continue;
        }
        if(path.parse(files[i]).name.toLocaleLowerCase() == prjname.toLocaleLowerCase()){
            return fileos.getPath(currpath + "/" + files[i]);
        }
    }
    return "";
}
function _parseVcProjectAuto(currpath, prjname) {
    var prjfile = _findvcprjfile(currpath,prjname);
    if (prjfile == "") {
        return "";
    }

    return _parseVCProjectDepend(currpath, prjfile)
        + _parseDependVCProject(currpath, prjname);
}
function _fileVersionInfo(currpath, linuxpath, scanpath, name) {
    var result = new Array();

    var files = fs.readdirSync(fileos.getPath(currpath + "/" + scanpath + "/"));
    for (var i = 0; i < files.length; i++) {
        if (files[i].substring(0, 1) == "." || files[i] == "_Output" || files[i] == "__Compile" || files[i] == "Lib" || files[i] == "mk" || files[i] == "packages") {
            continue;
        }
        var info = fs.statSync(fileos.getPath(currpath + "/" + scanpath + "/" + files[i]));
        if (info.isDirectory()) {
            result = result.concat(_fileVersionInfo(currpath, linuxpath, scanpath + "/" + files[i], files[i]));
            continue;
        }
        if (path.parse(files[i]).base.toLocaleLowerCase() != "version.tpl" || !name || name == "") {
            continue;
        }

        var prjpath = "";
        if (fs.existsSync(fileos.getPath(currpath + "/include/" + name + "/Version.tpl"))) {
            prjpath = "/include/" + name + "/Version.tpl";
        } else if (fs.existsSync(fileos.getPath(currpath + "/" + name + "/Version.tpl"))) {
            prjpath = "/" + name + "/Version.tpl";
        }
        if (prjpath == "") {
            continue;
        }

        var config = JSON.parse(fileos.readFileSync(fileos.getPath(currpath + prjpath)));

        var obj = {};
        obj.name = name;
        obj.version = config.productVersion;
        obj.summary = config.summary;
        obj.definitions = config.definitions;
        obj.dependency = config.dependency;
        obj.description = config.description;
        obj.linux = (linuxpath && fs.existsSync(fileos.getPath(linuxpath + "/" + prjpath)));
        obj.autodepend = _parseVcProjectAuto(currpath, name);
        obj.includrootpath = config.includrootpath ? config.includrootpath : false;
        result = result.concat(obj);
    }

    return result;
}
function _loadpkt(inputfile, outputfile) {
    if (fs.existsSync(fileos.getPath(inputfile))) {
        var pktobj = JSON.parse(fileos.readFileSync(fileos.getPath(inputfile)));
        var linuxpath = pktobj.linuxpath;
        var currpath = pktobj.currpath;


        var pktobj = {};
        pktobj.result = _fileVersionInfo(currpath, linuxpath, "", "");

        fileos.writeFileSync(outputfile, JSON.stringify(pktobj, null, 2));
    }
    fileos.rmfile(inputfile);
}

function _loadfilepkt(inputfile, outputfile) {
    if (fs.existsSync(fileos.getPath(inputfile))) {
        var pktobj = JSON.parse(fileos.readFileSync(fileos.getPath(inputfile)));
        var currpath = pktobj.currpath;
         var obj = {};
        obj.name = path.parse(path.parse(currpath).dir).base;
        obj.version = path.parse(currpath).base;
        if (fs.existsSync(fileos.getPath(`${currpath}/Version.tpl`))) {
            var config = JSON.parse(fileos.readFileSync(fileos.getPath(`${currpath}/Version.tpl`)));
        }
        obj.summary = config && config.summary ? config.summary : "";
        obj.definitions = config && config.definitions ? config.definitions : "";
        obj.dependency = config && config.dependency ? config.dependency : "";
        obj.description = config && config.description ? config.description : "";
        obj.includrootpath = config && config.includrootpath ? config.includrootpath : false;
        
        fileos.writeFileSync(outputfile, obj.name == "" ? "" : JSON.stringify(obj, null, 2));
    }
    fileos.rmfile(inputfile);
}

function _filesetpktinfo(inputfile) {
    if (fs.existsSync(fileos.getPath(inputfile))) {
        var pktobj = JSON.parse(fileos.readFileSync(fileos.getPath(inputfile), currpath));
        var currpath = pktobj.currpath;

        var version = {};
        version.productVersion = pktobj.result.version;
        version.summary = pktobj.result.summary;
        version.definitions = pktobj.result.definitions;
        version.dependency = pktobj.result.dependency ? pktobj.result.dependency : "";
        version.description = pktobj.result.description;
        version.includrootpath = pktobj.result.includrootpath;

         if (fs.existsSync(fileos.getPath(currpath))) 
         {
            fileos.writeFileSync(fileos.getPath(currpath + "/Version.tpl"), JSON.stringify(version, null, 2));
         }        
    }
    fileos.rmfile(inputfile);
}

function _packgeNugetPath(inputfile) {
    if (fs.existsSync(fileos.getPath(inputfile))) {
        var pktobj = JSON.parse(fileos.readFileSync(fileos.getPath(inputfile)));
        var packgdir = pktobj.currpath;

        var includepath = "";
        var win32libpath = "";
        var linuxlibpath = "";
        var binpath = new Array();
        if (fs.existsSync(fileos.getPath(`${packgdir}/Include`))) includepath = fileos.getPath(`${packgdir}/Include`);
        if (fs.existsSync(fileos.getPath(`${packgdir}/Lib/win32`))) win32libpath = fileos.getPath(`${packgdir}/Lib/win32`);
        if (fs.existsSync(fileos.getPath(`${packgdir}/Bin/win32`))) binpath = binpath.concat({path:fileos.getPath(`${packgdir}/Bin/win32/`),pathname:"Bin\\win32"});
        if (fs.existsSync(fileos.getPath(`${packgdir}/Lib/x86`))) linuxlibpath = fileos.getPath(`${packgdir}/Lib/x86`);
        if (fs.existsSync(fileos.getPath(`${packgdir}/Bin/x86`))) binpath = binpath.concat({path:fileos.getPath(`${packgdir}/Bin/x86/`),pathname:"Bin\\x86"});
        if (fs.existsSync(fileos.getPath(`${packgdir}/Doc`))) binpath = binpath.concat({path:fileos.getPath(`${packgdir}/Doc`),pathname:"Doc"});
        if (fs.existsSync(fileos.getPath(`${packgdir}/Demo`))) binpath = binpath.concat({path:fileos.getPath(`${packgdir}/Demo`),pathname:"Demo"});

        _buildNugetPacket(pr.cwd(),packgdir,packgdir,packgdir,{ name: pktobj.result.name ,includrootpath:pktobj.result.includrootpath},includepath,win32libpath,linuxlibpath,binpath);
    }
    fileos.rmfile(inputfile);
}
var arg = process.argv.splice(2);
if (arg[0].toLocaleLowerCase() == "loaddepend") {
    _loadpkt(arg[1], arg[2]);
} else if (arg[0].toLocaleLowerCase() == "setdepend") {
    _setpktinfo(arg[1]);
} else if (arg[0].toLocaleLowerCase() == "publishdepend") {
    _packgeNuget(arg[1]);
} else if (arg[0].toLocaleLowerCase() == "fileloaddepend") {
    _loadfilepkt(arg[1], arg[2]);
} else if (arg[0].toLocaleLowerCase() == "setfilepacketinfo") {
    _filesetpktinfo(arg[1]);
} else if (arg[0].toLocaleLowerCase() == "packetfilepacketinfo") {
    _packgeNugetPath(arg[1]);
}
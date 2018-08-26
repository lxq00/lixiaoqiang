const fs = require("fs");
const os = require("os");
const cp = require('child_process');
const pr = require('process');
const path = require('path');
const crypto = require('crypto')
const log = require("../js/pktlog.js");
const fileos = require('../js/fileos.js');
const Version = require("../js_v3/version.js");
const Event = require("../js_v3/event.js")
const oszip = require("../js/oszip.js");


module.exports = class Sln {
    about() {
        log.printdebuginfo(2, "build/b makefile/slnfile", "根据解决方案生成相应依赖");
        //log.printdebuginfo(2, "depend/d slnFileName prjFileName", "对工程文件按照解决方案依赖关联相应库");
    }
    doobj(args) {
        if (args[1] && args[2] && args[3] && (args[1].toLocaleLowerCase() == "depend" || args[1].toLocaleLowerCase() == "d")) {
            var slnfilename = fileos.getPath(args[2]);
            var prjfilename = fileos.getPath(args[3]);
            var slnpath = path.parse(slnfilename).dir;
            var prjpath = path.parse(prjfilename).dir;
            var prjname = path.parse(prjfilename).base;

            this._buildProjectDepend(slnpath, slnfilename, prjpath, prjname, prjfilename);
        } else if (args[1] && (args[1].toLocaleLowerCase() == "build" || args[1].toLocaleLowerCase() == "b")) {
            log.log("Start Build SLN Project Depend!");
            this._buildSlnProjectDepend(args[2]);
            log.log("Success Build SLN Project Depend!");
        }
    }
    _getSlnFileName(currpath, slnfile) {
        var slnfilename = "";
        var info = fs.statSync(fileos.getPath(currpath + "/" + slnfile));
        if (info.isDirectory()) {
            var files = fs.readdirSync(fileos.getPath(currpath + "/" + slnfile));
            for (var i = 0; i < files.length; i++) {
                if (path.parse(files[i]).ext.toLocaleLowerCase() != ".sln") {
                    continue;
                }
                slnfilename = files[i];
            }
        } else {
            slnfilename = slnfile;
        }
        if (!fs.existsSync(fileos.getPath(currpath + "/" + slnfilename))) {
            return "";
        }
        return fileos.getPath(currpath + "/" + slnfilename);
    }
    _parseWin32SlnPrject(slnfile) {
        var slnprjectobj = new Array();
        var filedata = fs.readFileSync(slnfile).toString();
        while (true) {
            var startpos = filedata.indexOf(".vcxproj");
            if (startpos == -1) break;
            var prjdata = filedata.substring(0, startpos);
            var pstartpos = prjdata.lastIndexOf("\"");
            if (pstartpos == -1) break;

            slnprjectobj = slnprjectobj.concat(prjdata.substring(pstartpos + 1) + ".vcxproj");

            filedata = filedata.substring(startpos + 10);
        }

        return slnprjectobj;
    }
    _buildSlnProjectDepend(slnfile) {
        var currpath = pr.cwd();
        var slnfilename = this._getSlnFileName(currpath, slnfile ? slnfile : "");
        if (slnfilename == "") return;
        var prjobj = this._parseWin32SlnPrject(slnfilename);

        var args = new Array();
        args[1] = "depend";
        args[2] = slnfilename;
        for (var i = 0; i < prjobj.length; i++) {
            args[3] = path.parse(slnfilename).dir + "/" + prjobj[i];

            this.doobj(args);
        }
    }
    _buildProjectDependX86(slnpath, prjpath, prjname, dependobj) {
        var mkfile = this._findMkfile(path.parse(fileos.getPath(prjname)).name, slnpath);
        if (!fs.existsSync(fileos.getPath(mkfile))) {
            return;
        }
        var currpath = pr.cwd();
        var relatinvepath = path.relative(slnpath, currpath);

        var nugetpkg = new Array();
        if (fs.existsSync(fileos.getPath(`${prjpath}/packages.config`))) {
            nugetpkg = this._parseProjectDependx86(slnpath, `${prjpath}/packages.config`, relatinvepath);
        }

        this._writeNugetToMkfile(slnpath, mkfile, nugetpkg, dependobj);
    }
    _writeNugetToMkfile(slnpath, mkfile, nugetpkg, dependobj) {
        var filedata = fs.readFileSync(mkfile).toString();

        //insert nuget pkt
        {
            var extmkstart = "#UserDefined Option";

            var extfilestartpos = filedata.indexOf(extmkstart);
            if (extfilestartpos == -1) {
                return;
            }
            var newfiledata = filedata.substring(0, extfilestartpos + extmkstart.length) + "\r\n";

            var tmpbuffer = filedata.substring(extfilestartpos + extmkstart.length);
            var extfileendpos = tmpbuffer.indexOf("-include userdefinedOption.mk");
            if (extfileendpos == -1) {
                return;
            }
            var tmpendbuffer = tmpbuffer.substring(extfileendpos);

            for (var i = 0; i < nugetpkg.length; i++) {
                newfiledata += `include ${nugetpkg[i]}\r\n `;
            }
            newfiledata += tmpendbuffer;
            filedata = newfiledata;
        }

        {
            var extmkstart = "#AutoAddOtherDefineStart";

            var extfilestartpos = filedata.indexOf(extmkstart);
            if (extfilestartpos == -1) {
                return;
            }
            var newfiledata = filedata.substring(0, extfilestartpos + extmkstart.length) + "\r\n";

            var tmpbuffer = filedata.substring(extfilestartpos + extmkstart.length);
            var extfileendpos = tmpbuffer.indexOf("#AotoAddOtherDefineEnd");
            if (extfileendpos == -1) {
                return;
            }
            var tmpendbuffer = tmpbuffer.substring(extfileendpos);

            var ldliblink = "";
            var ldlibdbglink = "";
            for (var i = 0; i < dependobj.length; i++) {
                if (fs.existsSync(fileos.getPath(`${slnpath}/${dependobj[i].libname}`))) {
                    ldliblink += `-L $(PRJ_LIBDIR)/$(PLATFORM)/${dependobj[i].libname} -Wl,-rpath=$(PRJ_LIBDIR)/$(PLATFORM)/${dependobj[i].libname} -l${dependobj[i].libname}\\\r\n`;
                    ldlibdbglink += `-L $(PRJ_LIBDIR)/$(PLATFORM)/${dependobj[i].libname} -Wl,-rpath=$(PRJ_LIBDIR)/$(PLATFORM)/${dependobj[i].libname} -l${dependobj[i].libname}_debug\\\r\n`;
                }
            }
            newfiledata += "LDLIBS = " + ldliblink + "\r\n\r\n";
            newfiledata += "LDLIBS_DBG = " + ldlibdbglink + "\r\n\r\n";

            newfiledata += tmpendbuffer;
            filedata = newfiledata;
        }

        fs.writeFileSync(mkfile, filedata);
    }
    _parseNugetPacket(filename) {
        var filedata = fileos.readFileSync(filename).toString();

        var nugetnamestart = "id=\"";
        var nugetverstart = "version=\"";

        var nugetpkg = new Array();
        while (true) {
            var namestartpos = filedata.indexOf(nugetnamestart);
            if (namestartpos == -1) {
                break;
            }
            var data = filedata.substring(namestartpos + nugetnamestart.length);
            var nameendpos = data.indexOf("\"");
            if (nameendpos == -1) {
                break;
            }
            var name = data.substring(0, nameendpos);
            var versionstart = data.substring(nameendpos + 1);
            var versionstartpos = data.indexOf(nugetverstart);
            if (versionstartpos == -1) {
                break;
            }
            data = data.substring(versionstartpos + nugetverstart.length);
            var versionendpos = data.indexOf("\"");
            if (versionendpos == -1) {
                break;
            }
            var version = data.substring(0, versionendpos);

            var nuget = {};
            nuget.name = name;
            nuget.version = version;
            nugetpkg = nugetpkg.concat(nuget);

            filedata = data.substring(versionendpos + 1);
        }

        return nugetpkg;
    }
    _parseProjectDependx86(currpath, nugetfile, relatinvepath) {
        var nugetpkgarray = this._parseNugetPacket(fileos.getPath(nugetfile));
        var nugetpkg = new Array();
        for (var i = 0; i < nugetpkgarray.length; i++) {
            var extmkfile = fileos.getPath("${PRJ_PATH}/" + `${relatinvepath}packages/${nugetpkgarray[i].name}.${nugetpkgarray[i].version}/build/native/${nugetpkgarray[i].name}.mk`);
            nugetpkg = nugetpkg.concat(extmkfile);
        }

        return nugetpkg;
    }
    _buildProjectDepend(slnpath, slnfilename, prjpath, prjname, prjfilename) {
        var dependobj = this._parseSlnDepend(slnfilename, prjname, dependobj);
        if (os.platform() == "win32") {
            this._WriteDependToVCProjectFile(prjpath, dependobj);
        } else {
            this._buildProjectDependX86(slnpath, prjpath, prjname, dependobj);
        }
    }
    _findMkfile(name, slnpath) {
        var prjpath = fileos.getPath(`${slnpath}/${name}`);
        if (!fs.existsSync(prjpath)) {
            prjpath = slnpath;
        }
        var namemkfile = "";
        var defaultmkfile = "";
        var files = fs.readdirSync(prjpath);
        for (var i = 0; i < files.length; i++) {
            if (files[i].substring(0, 1) == "." || files[i] == "_Output" || files[i] == "__Compile" || files[i] == "Include" || files[i] == "Lib" || files[i] == "mk") {
                continue;
            }
            var info = fs.statSync(fileos.getPath(prjpath + "/" + files[i]));
            if (info.isDirectory()) {
                continue;
            }
            if (path.parse(files[i]).ext.toLocaleLowerCase() != ".mk") {
                continue;
            }

            defaultmkfile = `${prjpath}/${files[i]}`;
            if (path.parse(files[i]).name.toLocaleLowerCase() == name.toLocaleLowerCase()) {
                namemkfile = `${prjpath}/${files[i]}`;
            }
        }

        return namemkfile != "" ? namemkfile : defaultmkfile;
    }
    _WriteDependToVCProjectFile(prjpath, dependobj) {
        var dependstr = "";
        for (var i = 0; i < dependobj.length; i++) {
            dependstr += dependobj[i].name + ";";
        }
        var nowsha1 = crypto.createHash('md5').update(dependstr).digest('hex');


        var prjdependfile = path.normalize(`${prjpath}/packages-depend.targets`);
        if (!fs.existsSync(prjdependfile)) {
            return;
        }
        var filedata = fs.readFileSync(prjdependfile).toString();

        var dependstartstr = `<Target Name="depend_sha1_`;
        var dependindexpos = filedata.indexOf(dependstartstr);
        if (dependindexpos == -1) {
            return;
        }
        var tmpdata = filedata.substring(dependindexpos + dependstartstr.length);
        var dependendpos = tmpdata.indexOf(`"></Target>`);
        if (dependendpos == -1) {
            return;
        }
        var dependsha1 = tmpdata.substring(0, dependendpos);

        if (nowsha1 == dependsha1) {
            //return;
        }
        filedata = filedata.substring(0, dependindexpos) + dependstartstr + nowsha1 + tmpdata.substring(dependendpos);
        filedata = this._parseDependLibData(dependobj, filedata);

        fs.writeFileSync(prjdependfile, filedata);
    }
    _parseDependLibData(dependobj, dependfiledata) {
        var dependlibstr = "", dependlibstr_debug = "";
        {
            for (var i = 0; i < dependobj.length; i++) {
                dependlibstr += `$(SolutionDir)/Lib/Win32/${dependobj[i].libname}/${dependobj[i].libname}.lib;`;
                dependlibstr_debug += `$(SolutionDir)/Lib/Win32/${dependobj[i].libname}/${dependobj[i].libname}_debug.lib;`;
            }
        }
        dependlibstr += `%(AdditionalDependencies)`;
        dependlibstr_debug += `%(AdditionalDependencies)`;

        dependfiledata = this._repleaseLibStr(dependfiledata, `<ItemDefinitionGroup Label="Win32 and dynamic and Debug"`, dependlibstr_debug);
        dependfiledata = this._repleaseLibStr(dependfiledata, `<ItemDefinitionGroup Label="Win32 and dynamic and Release"`, dependlibstr);

        return dependfiledata;
    }
    _repleaseLibStr(filedata, debugflag, libpath) {
        var debugstart = filedata.indexOf(debugflag);
        if (debugstart == -1) {
            return filedata;
        }
        var debugstring = filedata.substring(debugstart + debugflag.length);
        var startposflag = `<AdditionalDependencies>`;
        var debugstrstartpos = debugstring.indexOf(`<AdditionalDependencies>`);
        if (debugstrstartpos == -1) {
            return filedata;
        }
        var debugstrendpos = debugstring.indexOf(`</AdditionalDependencies>`);
        if (debugstrendpos == -1) {
            return filedata;
        }

        return filedata.substring(0, debugstart + debugflag.length) + debugstring.substring(0, debugstrstartpos + startposflag.length) + libpath + debugstring.substring(debugstrendpos);
    }
    _parseDependLibName(slnfilename, dependobj) {
        var dependObj = new Array();
        var filedata = fs.readFileSync(slnfilename).toString();
        for (var i = 0; i < dependobj.length; i++) {
            var findstr = `"{${dependobj[i]}}"`;
            var dependpos = filedata.indexOf(findstr);
            if (dependpos == -1) {
                break;
            }
            var prjdata = filedata.substring(0, dependpos);
            var startposflag = `}") = "`;
            var prjnamestartpos = prjdata.lastIndexOf(startposflag);
            if (prjnamestartpos == -1) {
                break;
            }
            var prjnamestr = prjdata.substring(prjnamestartpos + startposflag.length);
            var prjnameendpos = prjnamestr.indexOf(`",`);
            if (prjnameendpos == -1) {
                break;
            }

            var libname = prjnamestr.substring(0, prjnameendpos);

            dependObj = dependObj.concat({ name: dependobj[i], libname: libname });
        }


        return dependObj;
    }
    _parsePrjDependObj(slnfilename, prjdata) {
        var dependobj = new Array();
        var startpos = prjdata.indexOf("postProject");
        if (startpos == -1) {
            return dependobj;
        }
        prjdata = prjdata.substring(startpos);
        while (true) {
            var pos = prjdata.indexOf("{");
            if (pos == -1) {
                break;
            }
            var endpos = prjdata.indexOf("} = {");
            if (endpos == -1) {
                break;
            }
            dependobj = dependobj.concat(prjdata.substring(pos + 1, endpos));

            var dependenddata = prjdata.substring(endpos + 5);
            endpos = dependenddata.indexOf("}");
            if (endpos == -1) {
                break;
            }
            prjdata = dependenddata.substring(endpos + 1);
        }
        return this._parseDependLibName(slnfilename, dependobj);
    }
    _parseSlnDepend(slnfilename, prjfilename) {
        var filedata = fs.readFileSync(slnfilename).toString();
        var prjpos = filedata.indexOf(prjfilename);
        if (prjpos == -1) {
            return { sha1: "", depend: [] };
        }
        var prjdata = filedata.substring(prjpos + prjfilename.length);
        var prjposend = prjdata.indexOf("EndProject");
        if (prjposend == -1) {
            return { sha1: "", depend: [] };
        }
        prjdata = prjdata.substring(0, prjposend);

        return this._parsePrjDependObj(slnfilename, prjdata);
    }
}
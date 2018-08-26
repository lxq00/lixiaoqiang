const fs = require("fs");
const os = require("os");
const cp = require('child_process');
const pr = require('process');
const path = require('path');
const log = require("../js/pktlog.js");
const fileos = require("../js/fileos.js");

module.exports = class CopyDll {
    copy(currpath, file, todir) {
        var allsofile = this._scanfAllSoFileInCurrPath(currpath,todir);
        var havecopyfile = {};
        this._copyFile(currpath, file, todir, allsofile, havecopyfile);
    }

    _copyFile(currpath, file, topath, allsofile, havecopyfile) {
        var filename = path.parse(file).base;
        if (havecopyfile[filename]) {
            return havecopyfile;
        }
        havecopyfile[filename] = true;
        if (this._isExeOrSharedLib(file)) {
            var dependfile = this._scanfFileDependUserLib(file);
            for (var i = 0; i < dependfile.length; i++) {
                var libname = this._strstrip(dependfile[i]);
                if (!allsofile[libname]) {
                    continue;
                }
                havecopyfile = this._copyFile(currpath, allsofile[libname], topath, allsofile, havecopyfile);
            }
            fileos.copy(file, topath);
        }

        fileos.copy(file, topath);

        return havecopyfile;
    }

    _strstrip(str) {
        return str.replace(/(^\s*)|(\s*$)/g,"");
    }

    _isExeOrSharedLib(file) {
        var evendata = cp.execSync(`file ${file} 2>/dev/null`).toString();

        return evendata.indexOf(`LSB executable,`) != -1 || evendata.indexOf(`LSB shared object,`) != -1;
    }

    _scanfAllSoFileInCurrPath(currpath,todir) {
        var evendata = cp.execSync(`find ${currpath} -name *.so* 2>/dev/null`).toString();
        var sofilearray = evendata.split('\n');
        var allfile = {};
        for (var i = 0; i < sofilearray.length; i++) {
            if(path.parse(sofilearray[i]).dir != todir){
                allfile[path.parse(sofilearray[i]).base] = sofilearray[i];
            }
        }
        return allfile;
    }

    _scanfFileDependUserLib(file) {
        if (!this._isExeOrSharedLib(file)) {
            return new Array();
        }
        var evendata = cp.execSync(`ldd ${file} 2>/dev/null`).toString();
        var sofilearray = evendata.split('\n');

        var sofile = new Array();
        for (var i = 0; i < sofilearray.length; i++) {
            var filename = sofilearray[i];

            var startflag = ` => `;

            var startpos = filename.indexOf(startflag);
            if (startpos == -1) break;
            var filename = filename.substring(0, startpos);
            sofile = sofile.concat(filename);
        }

        return sofile;
    }

}
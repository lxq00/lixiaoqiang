const fs = require("fs");
const os = require("os");
const cp = require('child_process');
const path = require('path');
const iconv = require('../iconv-lite');

let fileos=module.exports;

Date.prototype.format = function(fmt){
    var o = {
        "M+" : this.getMonth()+1,                 //�·�
        "d+" : this.getDate(),                    //��
        "h+" : this.getHours(),                   //Сʱ
        "m+" : this.getMinutes(),                 //��
        "s+" : this.getSeconds(),                 //��
        "q+" : Math.floor((this.getMonth()+3)/3), //����
        "S"  : this.getMilliseconds()             //����
    };

    if(/(y+)/.test(fmt))
        fmt=fmt.replace(RegExp.$1, (this.getFullYear().toString()).substr(4 - RegExp.$1.length));
    for(var k in o)
        if(new RegExp("("+ k +")").test(fmt))
            fmt = fmt.replace(RegExp.$1, (RegExp.$1.length==1) ? (o[k]) : (("00"+ o[k]).substr((""+ o[k]).length)));
    return fmt;
};

fileos.split = function()
{
    if(os.platform() == "win32")
    {
        return "\\";
    }
    else if(os.platform() == "linux")
    {
        return "/"
    }

    return "";
}

fileos.date = function(fmt)
{
    var date = new Date();

    return date.format(fmt);
}

fileos.getPath=function(path)
{
	if(os.platform() == "win32")
    {
        while(path.indexOf("/") != -1)
		{
			path = path.replace("/","\\");
		}
    }
    else if(os.platform() == "linux")
    {
		while(path.indexOf("\\") != -1)
		{
			path = path.replace("\\","/");
		}
    }

	return path;
}
fileos.execSync = function(cmdstr)
{
    cp.execSync(getPath(cmdstr));
}
fileos.copy = function(src,dst)
{
    if (os.platform() == "win32")
    {
        cp.execSync("xcopy /F /Y \""+fileos.getPath(src)+"\" \""+fileos.getPath(dst)+"\" 2>nul");
    }
    else if (os.platform() == "linux")
    {
        cp.execSync("\cp -rfd "+fileos.getPath(src)+" \""+fileos.getPath(dst)+"\" 2>/dev/null");
    }
}
fileos.copyDir = function(src, dst)
{
    if (os.platform() == "win32")
    {
        cp.execSync("xcopy /S /F /Y \""+fileos.getPath(src)+"\" \""+fileos.getPath(dst)+"\" 2>nul");
    }
    else if (os.platform() == "linux")
    {
        cp.execSync("\cp -rfd "+fileos.getPath(src)+" \""+fileos.getPath(dst)+"\" 2>/dev/null");
    }
}

fileos.rmfile = function(path,checkexist = true)
{
    if(checkexist && !fs.existsSync(fileos.getPath(path)))
    {
        return;
    }

    if (os.platform() == "win32")
    {
        cp.execSync("del /Q /F \""+fileos.getPath(path)+"\" 2>nul");
    }
    else if (os.platform() == "linux")
    {
        cp.execSync("rm -rfd \""+fileos.getPath(path)+"\" 2>/dev/null");
    }
}
fileos.rmdir = function (path)
{
    if(!fs.existsSync(path))
    {
        return;
    }

    if (os.platform() == "win32")
    {
        cp.execSync("rmdir /Q /S \""+fileos.getPath(path)+"\" 2>nul");
    }
    else if (os.platform() == "linux")
    {
        cp.execSync("rm -rfd \""+fileos.getPath(path)+"\" 2>/dev/null");
    }
}
fileos.rmdirforce = function (path)
{
    while(fs.existsSync(fileos.getPath(path)))
    {
        fileos.rmdir(fileos.getPath(path));
    }
}
fileos.move=function(oldfile,newfile){
    if(!fs.existsSync(oldfile))
    {
        return;
    }

    if (os.platform() == "win32")
    {
        cp.execSync("move /Y \""+fileos.getPath(oldfile)+"\" \""+fileos.getPath(newfile)+"\" 2>nul");
    }
    else if (os.platform() == "linux")
    {
        cp.execSync("mv -f \""+fileos.getPath(oldfile)+"\" \""+fileos.getPath(newfile)+"\" 2>/dev/null");
    }
}
fileos.writeFileSync=function(filename,data){
    var utf_8 = iconv.encode(data,"gb2312");

    fs.writeFileSync(fileos.getPath(filename),utf_8);
}
fileos.readFileSync=function(filename){
    var data = fs.readFileSync(fileos.getPath(filename));

    return iconv.decode(data,"gb2312");
}

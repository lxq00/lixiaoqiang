const fs = require("fs");
const os = require("os");


let version=module.exports;

version.buildFullVersion = function(intputver)
{
    inputverarry = intputver.split(".");
    if(inputverarry.length < 1)
    {
        inputverarry[0] = "0";
    }
    if(inputverarry.length < 2)
    {
        inputverarry[1] = "0";
    }
    if(inputverarry.length < 3)
    {
        inputverarry[2] = "0";
    } 
    for(var i = 0;i < 3;i ++)
    {
        inputverarry[i] = parseInt(inputverarry[i]);
    }

    return inputverarry;
}
version.cmpVersion = function(ver1,ver2)
{
    ver1arry = version.buildFullVersion(ver1);
    ver2arry = version.buildFullVersion(ver2);

    for(var i = 0;i < 3 ;i ++)
    {
        if(ver1arry[i] > ver2arry[i])
        {
            return 1;
        }
        else if(ver1arry[i] < ver2arry[i])
        {
            return -1;
        }
    }

    return 0;
}

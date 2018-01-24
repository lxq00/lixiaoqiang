@ECHO OFF

SET CURRPATH=%~f0
SET CURRFILEPATH=%CURRPATH:~0,-12%

cd /d %CURRFILEPATH%

node \\192.168.0.10\vgsii_depends\__VGSII__GIT_Tool\vgsii_pkt_tools_2.0\mk\pkt\pkt_v3.js %1 %2 %3 %4

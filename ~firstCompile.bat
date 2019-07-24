@echo off

"mk/pkt/tool/7za.exe" x boost/include/boost.zip -y -oboost/include/
"mk/pkt/tool/7za.exe" x boost/Lib/win32.zip -y -oboost/Lib/
"mk/pkt/tool/7za.exe" x ffmpeg/Lib/win32.zip -y -offmpeg/Lib/
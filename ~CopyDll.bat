@echo off
for /f "delims=" %%i in ('dir "Lib\win32" /b') do (xcopy "Lib\win32\%%i\*" "Bin\win32\" /s /e /Y)


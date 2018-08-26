@echo off
set currgit=%git%
set currbuild2015=%build2015%
set currsetupfactory=%setupfactory%
set currnuget=%nuget%

if "%currgit%" == "" (
	For /f "tokens=1* delims=_" %%1 in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\GitForWindows" /v "InstallPath"') Do (
		For /f "tokens=1*" %%3  in ("%%~2") Do (
			SET currgit=%%4
		)
	)
	SET currgit="C:\Program Files\Git"
)


if "%currbuild2015%"=="" (
	For /f "tokens=1* delims=_" %%1 in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\MSBuild\14.0" /v "MSBuildOverrideTasksPath"') Do (
		For /f "tokens=1*" %%3  in ("%%~2") Do (
			SET currbuild2015=%%4MSBuild.exe
		)
	)
)

if "%currsetupfactory%" == "" (
	SET currsetupfactory="C:\Program Files (x86)\Setup Factory 9\SUFDesign.exe"
)

echo {"git":"%currgit%","msbuild":"%currbuild2015%","setupfactory":"%currsetupfactory%","nuget":"%currnuget% "}

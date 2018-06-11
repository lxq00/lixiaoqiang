@echo off

SET GIT_PATH=""
For /f "tokens=1* delims=_" %%1 in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\GitForWindows" /v "InstallPath"') Do (
	For /f "tokens=1*" %%3  in ("%%~2") Do (
		SET GIT_PATH=%%4
	)
)


SET VS2015_PATH=""
if "%PROCESSOR_ARCHITECTURE%"=="x86" (
	For /f "tokens=1* delims=_" %%1 in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\14.0\Setup\VS" /v "EnvironmentPath"') Do (
		For /f "tokens=1*" %%3  in ("%%~2") Do (
			SET VS2015_PATH=%%4
		)
	)
) else (
	For /f "tokens=1* delims=_" %%1 in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\VisualStudio\14.0\Setup\VS" /v "EnvironmentPath"') Do (
		For /f "tokens=1*" %%3  in ("%%~2") Do (
			SET VS2015_PATH=%%4
		)
	)
)


SET VS2012_PATH=""
if "%PROCESSOR_ARCHITECTURE%"=="x86" (
	For /f "tokens=1* delims=_" %%1 in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\11.0\Setup\VS" /v "EnvironmentPath"') Do (
		For /f "tokens=1*" %%3  in ("%%~2") Do (
			SET VS2012_PATH=%%4
		)
	)
) else (
	For /f "tokens=1* delims=_" %%1 in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\VisualStudio\11.0\Setup\VS" /v "EnvironmentPath"') Do (
		For /f "tokens=1*" %%3  in ("%%~2") Do (
			SET VS2012_PATH=%%4
		)
	)
)

SET VS2008_PATH=""
if "%PROCESSOR_ARCHITECTURE%"=="x86" (
	For /f "tokens=1* delims=_" %%1 in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\9.0\Setup\VS" /v "EnvironmentPath"') Do (
		For /f "tokens=1*" %%3  in ("%%~2") Do (
			SET VS2008_PATH=%%4
		)
	)
) else (
	For /f "tokens=1* delims=_" %%1 in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\VisualStudio\9.0\Setup\VS" /v "EnvironmentPath"') Do (
		For /f "tokens=1*" %%3  in ("%%~2") Do (
			SET VS2008_PATH=%%4
		)
	)
)
if "%build2015%"=="" (
	For /f "tokens=1* delims=_" %%1 in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\MSBuild\14.0" /v "MSBuildOverrideTasksPath"') Do (
		For /f "tokens=1*" %%3  in ("%%~2") Do (
			SET build2015=%%4MSBuild.exe
		)
	)
)

echo {"git":"%GIT_PATH%","vs2015":"%VS2015_PATH%","vs2012":"%VS2012_PATH%","vs2008":"%VS2008_PATH%","msbuild":"%build2015%"}

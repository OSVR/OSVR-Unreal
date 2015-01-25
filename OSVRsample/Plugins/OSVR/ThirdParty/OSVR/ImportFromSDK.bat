@echo off

set PRJ_ROOT=%~dp0\..\..\..

IF NOT EXIST "%PRJ_ROOT%\OSVRsample" goto ERROR_WRONG_PROJ_DIR

IF %1.==. (
	set /p osvr32bit=Type OSVR 32bit SDK root dir:
) ELSE (
	set osvr32bit=%~1
)
IF %2.==. (
	set /p osvr64bit=Type OSVR 64bit SDK root dir:
) ELSE (
	set osvr64bit=%~2
)

REM includes
RMDIR /S /Q "%~dp0\include"
XCOPY "%osvr32bit%\include\osvr\ClientKit" "%~dp0\include\osvr\ClientKit" /S /I /y
XCOPY "%osvr32bit%\include\osvr\Util" "%~dp0\include\osvr\Util" /S /I /y

REM 32bit/64bit dlls
RMDIR /S /Q "%~dp0\bin"
XCOPY "%osvr32bit%\bin\osvrClient.dll" "%~dp0\bin\Win32\"
XCOPY "%osvr32bit%\bin\osvrClientKit.dll" "%~dp0\bin\Win32\"
XCOPY "%osvr32bit%\bin\osvrUtil.dll" "%~dp0\bin\Win32\"
XCOPY "%osvr32bit%\bin\osvrTransform.dll" "%~dp0\bin\Win32\"
XCOPY "%osvr64bit%\bin\osvrClient.dll" "%~dp0\bin\Win64\"
XCOPY "%osvr64bit%\bin\osvrClientKit.dll" "%~dp0\bin\Win64\"
XCOPY "%osvr64bit%\bin\osvrUtil.dll" "%~dp0\bin\Win64\"
XCOPY "%osvr64bit%\bin\osvrTransform.dll" "%~dp0\bin\Win64\"

REM 32bit/64bit libs
RMDIR /S /Q "%~dp0\lib"
XCOPY "%osvr32bit%\lib\osvrClientKit.lib" "%~dp0\lib\Win32\"
XCOPY "%osvr64bit%\lib\osvrClientKit.lib" "%~dp0\lib\Win64\"

REM sdk version
XCOPY "%osvr32bit%\osvr-ver.txt" "%~dp0\" /y
exit /b 0

:ERROR_WRONG_PROJ_DIR
echo Error: '%PRJ_ROOT%' is not the root project directory
pause
exit /b 1

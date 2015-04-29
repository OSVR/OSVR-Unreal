@echo off

set PRJ_ROOT=%~dp0
set DEST_ROOT=%~dp0OSVRUnreal\Plugins\OSVR\ThirdParty\OSVR

IF NOT EXIST "%DEST_ROOT%" goto ERROR_WRONG_PROJ_DIR

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

rem Get rid of the old
RMDIR /S /Q "%DEST_ROOT%\include"
RMDIR /S /Q "%DEST_ROOT%\bin"
RMDIR /S /Q "%DEST_ROOT%\lib"
del "%DEST_ROOT%/*.txt"

call :copy_arch_indep %osvr32bit% %DEST_ROOT%

call :copy_arch %osvr32bit% %DEST_ROOT% 32
call :copy_arch %osvr64bit% %DEST_ROOT% 64
goto :eof


:copy_arch_indep
rem Architecture-independent files
setlocal
set SRC=%1
set DEST_ROOT=%2
xcopy "%SRC%\include\osvr\ClientKit" "%DEST_ROOT%\include\osvr\ClientKit" /S /I /y
xcopy "%SRC%\include\osvr\Util" "%DEST_ROOT%\include\osvr\Util" /S /I /y
endlocal
goto :eof

:copy_arch
rem Architecture-dependent files
setlocal
set SRC=%1
set DEST_ROOT=%2
set BITS=%3 

copy "%SRC%\osvr-ver.txt" "%DEST_ROOT%\Win%BITS%osvr-ver.txt" /y

rem dlls
rem mkdir "%DEST_ROOT%\bin\Win%BITS%\"
for %%F in (%SRC%\bin\osvrClientKit.dll,%SRC%\bin\osvrClient.dll,%SRC%\bin\osvrUtil.dll,%SRC%\bin\osvrCommon.dll) do (
  xcopy %%F "%DEST_ROOT%\bin\Win%BITS%\" /Y
)

rem libs
for %%F in (%SRC%\lib\osvrClientKit.lib) do (
  xcopy %%F "%DEST_ROOT%\lib\Win%BITS%\" /Y
)
endlocal
goto :eof

:ERROR_WRONG_PROJ_DIR
echo Error: '%PRJ_ROOT%' is not the root project directory
pause
exit /b 1

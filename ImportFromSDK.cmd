@echo off

set PRJ_ROOT=%~dp0

set PLUGIN_ROOT=%~dp0OSVRUnreal\Plugins\OSVR
IF NOT EXIST "%PLUGIN_ROOT%" goto ERROR_WRONG_PROJ_DIR

set DEST_ROOT=%~dp0OSVRUnreal\Plugins\OSVR\Source\OSVRClientKit
IF NOT EXIST "%DEST_ROOT%" goto ERROR_WRONG_PROJ_DIR

set DEST_RM_ROOT=%~dp0OSVRUnreal\Plugins\OSVR\Source\OSVRRenderManager
IF NOT EXIST "%DEST_RM_ROOT%" goto ERROR_WRONG_PROJ_DIR

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

IF %3.==. (
	set /p rm32bit=Type DirectRender 32bit SDK root dir:
) ELSE (
	set rm32bit=%~3
)

# @todo copy the 64-bit version

rem Get rid of the old
RMDIR /S /Q "%DEST_ROOT%\include" > NUL
RMDIR /S /Q "%DEST_ROOT%\lib" > NUL
del "%DEST_ROOT%\*.txt" > NUL
RMDIR /S /Q "%DEST_RM_DIR%\include" > NUL
RMDIR /S /Q "%DEST_RM_DIR%\lib" > NUL
del "%DEST_RM_DIR%\*.txt" > NUL

call :copy_arch_indep %osvr32bit% %DEST_ROOT%
call :copy_arch_indep_rm %rm32bit% %DEST_RM_ROOT%

call :copy_arch %osvr32bit% %PLUGIN_ROOT% %DEST_ROOT% 32
call :copy_arch %osvr64bit% %PLUGIN_ROOT% %DEST_ROOT% 64
call :copy_arch_rm %rm32bit% %PLUGIN_ROOT% %DEST_RM_ROOT% 32
goto :eof


:copy_arch_indep_rm
rem Architecture-independent files
setlocal
set SRC=%1
set DEST_ROOT=%2
xcopy "%SRC%\include\osvr\RenderKit" "%DEST_ROOT%\include\osvr\RenderKit" /S /I /y
endlocal
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
set PLUGIN_ROOT=%2
set DEST_ROOT=%3
set BITS=%4

copy "%SRC%\osvr-ver.txt" "%DEST_ROOT%\Win%BITS%osvr-ver.txt" /y

rem One copy to the bin directory, for use in deployment.
call :copy_dll %SRC% %DEST_ROOT%\bin %BITS%

rem One copy to the plugin Binaries directory, for editor support.
call :copy_dll %SRC% %PLUGIN_ROOT%\Binaries %BITS%

rem libs
for %%F in (%SRC%\lib\osvrClientKit.lib) do (
  xcopy %%F "%DEST_ROOT%\lib\Win%BITS%\" /Y
)
endlocal
goto :eof

:copy_dll
rem Copy DLL files
setlocal
set SRC=%1
set DEST=%2
set BITS=%3
for %%F in (%SRC%\bin\osvrClientKit.dll,%SRC%\bin\osvrClient.dll,%SRC%\bin\osvrUtil.dll,%SRC%\bin\osvrCommon.dll) do (
  xcopy %%F "%DEST%\Win%BITS%\" /Y
)
endlocal
goto :eof

:copy_arch_rm
rem Architecture-dependent files
setlocal
set SRC=%1
set PLUGIN_ROOT=%2
set DEST_ROOT=%3
set BITS=%4

rem One copy to the bin directory, for use in deployment.
call :copy_dll_rm %SRC% %DEST_ROOT%\bin %BITS%

rem One copy to the plugin Binaries directory, for editor support.
call :copy_dll_rm %SRC% %PLUGIN_ROOT%\Binaries %BITS%

rem libs
for %%F in (%SRC%\lib\osvrRenderManager.lib) do (
  xcopy %%F "%DEST_ROOT%\lib\Win%BITS%\" /Y
)
endlocal
goto :eof

:copy_dll_rm
rem Copy DLL files
setlocal
set SRC=%1
set DEST=%2
set BITS=%3
for %%F in (%SRC%\bin\osvrRenderManager.dll) do (
  xcopy %%F "%DEST%\Win%BITS%\" /Y
)
endlocal
goto :eof

:ERROR_WRONG_PROJ_DIR
echo Error: '%PRJ_ROOT%' is not the root project directory
pause
exit /b 1

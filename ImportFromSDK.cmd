@echo off

set PRJ_ROOT=%~dp0

set PLUGIN_ROOT=%~dp0OSVRUnreal\Plugins\OSVR
IF NOT EXIST "%PLUGIN_ROOT%" goto ERROR_WRONG_PROJ_DIR

set DEST_ROOT=%~dp0OSVRUnreal\Plugins\OSVR\Source\OSVRClientKit
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

IF %3.==. (
	set /p osvrAndroid=Type OSVR Android SDK root dir:
) ELSE (
	set osvrAndroid=%~3
)

set rm32bit=%rm64bit%

rem Get rid of the old
RMDIR /S /Q "%DEST_ROOT%\include" > NUL
RMDIR /S /Q "%DEST_ROOT%\lib" > NUL
del "%DEST_ROOT%\*.txt" > NUL

call :copy_arch_indep %osvr32bit% %rm32bit% %DEST_ROOT%

call :copy_arch %osvr32bit% %rm32bit% %PLUGIN_ROOT% %DEST_ROOT% 32
call :copy_arch %osvr64bit% %rm64bit% %PLUGIN_ROOT% %DEST_ROOT% 64
call :copy_android %osvrAndroid% %PLUGIN_ROOT% %DEST_ROOT%

echo Note: The 32-bit target is not yet supported. Please use the 64-bit target only for now.

goto :eof


:copy_arch_indep
rem Architecture-independent files
setlocal
set SRC=%1
set SRC_RM=%src%
set DEST_ROOT=%3
xcopy %SRC%\include\osvr\ClientKit "%DEST_ROOT%\include\osvr\ClientKit" /S /I /y
xcopy %SRC%\include\osvr\Util "%DEST_ROOT%\include\osvr\Util" /S /I /y
xcopy %SRC%\include\osvr\Client "%DEST_ROOT%\include\osvr\Client" /S /I /y
xcopy %SRC%\include\osvr\Common "%DEST_ROOT%\include\osvr\Common" /S /I /y
xcopy %SRC_RM%\include\osvr\RenderKit "%DEST_ROOT%\include\osvr\RenderKit" /S /I /y
endlocal
goto :eof

:copy_android

set SRC=%1
set PLUGIN_ROOT=%2
set DEST_ROOT=%3
set SRC_LIB=%SRC%\NDK\osvr\builds\armeabi-v7a\lib

for %%F in (%SRC_LIB%\libcrystax.so,%SRC_LIB%\libfunctionality.so,%SRC_LIB%\libgnustl_shared.so,%SRC_LIB%\libjsoncpp.so,%SRC_LIB%\libosvrAnalysisPluginKit.so,%SRC_LIB%\libosvrClient.so,%SRC_LIB%\libosvrClientKit.so,%SRC_LIB%\libosvrCommon.so,%SRC_LIB%\libosvrConnection.so,%SRC_LIB%\libosvrJointClientKit.so,%SRC_LIB%\libosvrPluginHost.so,%SRC_LIB%\libosvrServer.so,%SRC_LIB%\libosvrUtil.so,%SRC_LIB%\libosvrVRPNServer.so) do (
  echo xcopy %%F "%DEST_ROOT%\bin\Android\armeabi-v7a\" /Y
  xcopy %%F "%DEST_ROOT%\bin\Android\armeabi-v7a\" /Y
)

endlocal
goto :eof

:copy_arch
rem Architecture-dependent files
setlocal
set SRC=%1
set SRC_RM=%2
set PLUGIN_ROOT=%3
set DEST_ROOT=%4
set BITS=%5

copy "%SRC%\osvr-ver.txt" "%DEST_ROOT%\Win%BITS%osvr-ver.txt" /y

rem One copy to the bin directory, for use in deployment.
call :copy_dll %SRC% %SRC_RM% %DEST_ROOT%\bin %BITS%

rem One copy to the plugin Binaries directory, for editor support.
call :copy_dll %SRC% %SRC_RM% %PLUGIN_ROOT%\Binaries %BITS%

rem libs
for %%F in (%SRC%\lib\osvrClientKit.lib) do (
  xcopy %%F "%DEST_ROOT%\lib\Win%BITS%\" /Y
)

for %%F in (%SRC_RM%\lib\osvrRenderManager.lib) do (
  xcopy %%F "%DEST_ROOT%\lib\Win%BITS%\" /Y
)

endlocal
goto :eof

:copy_dll
rem Copy DLL files
setlocal
set SRC=%1
set SRC_RM=%2
set DEST=%3
set BITS=%4
for %%F in (%SRC%\bin\osvrClientKit.dll,%SRC%\bin\osvrClient.dll,%SRC%\bin\osvrUtil.dll,%SRC%\bin\osvrCommon.dll) do (
  xcopy %%F "%DEST%\Win%BITS%\" /Y
)
for %%F in (%SRC_RM%\bin\osvrRenderManager.dll,%SRC_RM%\bin\d3dcompiler_47.dll,%SRC_RM%\bin\glew32.dll,%SRC_RM%\bin\SDL2.dll) do (
  echo xcopy %%F "%DEST%\Win%BITS%\" /Y
  xcopy %%F "%DEST%\Win%BITS%\" /Y
)
endlocal
goto :eof



:ERROR_WRONG_PROJ_DIR
echo Error: '%PRJ_ROOT%' is not the root project directory
pause
exit /b 1

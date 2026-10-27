@echo off
setlocal
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "ExcellentZipVS=%%i"
if not defined ExcellentZipVS exit /b 1
call "%ExcellentZipVS%\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
if errorlevel 1 exit /b %errorlevel%
if not exist Output\IconTests mkdir Output\IconTests
cl /nologo /std:c++20 /EHsc /W4 /WX /DUNICODE /D_UNICODE /D_WIN32_WINNT=0x0A00 /I ExcellentZip.UI.Modern scripts\test_icon_runtime.cpp ExcellentZip.UI.Modern\ExcellentZip.Icons.cpp /FoOutput\IconTests\ /FeOutput\IconTests\test_icon_runtime.exe /link shell32.lib shlwapi.lib advapi32.lib user32.lib ole32.lib oleaut32.lib runtimeobject.lib
if errorlevel 1 exit /b %errorlevel%
Output\IconTests\test_icon_runtime.exe
if errorlevel 1 exit /b %errorlevel%
rc /nologo /fo Output\IconTests\settings.res ExcellentZip.UI.Modern\SevenZip\CPP\7zip\UI\FileManager\SettingsPage.rc
if errorlevel 1 exit /b %errorlevel%
cl /nologo /std:c++20 /EHsc /W4 /WX /Wv:18 /DLANG /DUNICODE /D_UNICODE /D_WIN32_WINNT=0x0A00 /I ExcellentZip.UI.Modern /c ExcellentZip.UI.Modern\SevenZip\CPP\7zip\UI\FileManager\SettingsPage.cpp /FoOutput\IconTests\settings.obj

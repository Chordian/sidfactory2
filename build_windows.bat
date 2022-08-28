MSBuild.exe SIDFactoryII.sln -property:Configuration=Release -property:Platform=x86
IF ERRORLEVEL 1 EXIT 1

md artifacts
IF ERRORLEVEL 1 EXIT 1
md artifacts\color_schemes
IF ERRORLEVEL 1 EXIT 1
md artifacts\drivers
IF ERRORLEVEL 1 EXIT 1
md artifacts\music
IF ERRORLEVEL 1 EXIT 1
md artifacts\overlay
IF ERRORLEVEL 1 EXIT 1
md artifacts\documentation
IF ERRORLEVEL 1 EXIT 1

copy /Y release\SIDFactoryII.exe artifacts\
IF ERRORLEVEL 1 EXIT 1

copy /Y SIDFactoryII\config.ini artifacts\
IF ERRORLEVEL 1 EXIT 1
copy /Y SIDFactoryII\COPYING artifacts\
IF ERRORLEVEL 1 EXIT 1
copy /Y SIDFactoryII\SDL2.dll artifacts\
IF ERRORLEVEL 1 EXIT 1

copy /Y SIDFactoryII\color_schemes\*.* artifacts\color_schemes\
IF ERRORLEVEL 1 EXIT 1
copy /Y SIDFactoryII\drivers\*.* artifacts\drivers\
IF ERRORLEVEL 1 EXIT 1
xcopy /Y /S SIDFactoryII\music\*.* artifacts\music\
IF ERRORLEVEL 1 EXIT 1
copy /Y SIDFactoryII\overlay\*.* artifacts\overlay\
IF ERRORLEVEL 1 EXIT 1
copy /Y dist\documentation\*.* artifacts\documentation\

MSBuild.exe SIDFactoryII.sln -property:Configuration=Release -property:Platform=x86

md artifacts
md artifacts\color_schemes
md artifacts\drivers
md artifacts\music
md artifacts\overlay

copy /Y release\SIDFactoryII.exe artifacts\
copy /Y release\SF2Converter.exe artifacts\

copy /Y SIDFactoryII\config.ini artifacts\
copy /Y SIDFactoryII\COPYING artifacts\
copy /Y SIDFactoryII\SDL2.dll artifacts\

copy /Y SIDFactoryII\color_schemes\*.* artifacts\color_schemes\
copy /Y SIDFactoryII\drivers\*.* artifacts\drivers\
copy /Y SIDFactoryII\music\*.* artifacts\music\
copy /Y SIDFactoryII\overlay\*.* artifacts\overlay\

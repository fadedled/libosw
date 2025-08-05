@echo off

@echo Compile all source files
if not exist build (mkdir build)
pushd build
cl -O2 -c ..\src\*c ..\src\opengl\*c ..\src\platform\windows\*c -EHsc -I"..\include" -Fo.\
popd
@echo Generate lib\Osw.lib
if not exist lib (mkdir lib)
lib build\*.obj -out:lib\Osw.lib

:: test
:: cl -O2 -Wall -Tc main.c -I"..\include" -link User32.lib Opengl32.lib Xinput.lib Katsu.lib -SUBSYSTEM:CONSOLE -OUT:main.exe

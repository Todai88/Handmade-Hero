@echo off
call "D:\Schoolwork\VS2017\VC\Auxiliary\Build\vcvarsall" x64
w:
mkdir \build
pushd \build
cl -Zi ..\code\win32_handmade.cpp user32.lib gdi32.lib
popd
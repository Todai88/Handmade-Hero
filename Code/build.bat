@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall" x64
w:
mkdir \build
pushd \build
cl -Zi ..\code\win32_handmade.cpp user32.lib gdi32.lib
popd
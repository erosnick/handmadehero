@echo off

mkdir build
pushd build
cl /Zi -DUNICODE -D_UNICODE user32.lib Gdi32.lib /nologo /INCREMENTAL ../win32_handmade.cpp
popd
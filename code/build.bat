@echo off

mkdir build
pushd build
cl /Zi -DUNICODE -D_UNICODE user32.lib ../win32_handmade.cpp
popd
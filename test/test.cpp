#include <Windows.h>
#include <cstdio>

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    OutputDebugString(L"Hello World!");
    printf("This is a string.");
}
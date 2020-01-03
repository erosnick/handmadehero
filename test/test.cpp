#include <Windows.h>

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    OutputDebugString(L"Hello World!");

    MessageBox(nullptr, L"Hello", L"Hello", MB_OK);
}
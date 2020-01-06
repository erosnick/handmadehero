#include <Windows.h>
#include <cstdio>

struct Projectile
{
	unsigned char IsThisOnFire;
	int Damage;
	int ParticlesPerSecond;
	short HowManyCooks;
};

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // This this remote server editing.
    OutputDebugString(L"Hello World!");
    printf("This is a string.");

    Projectile projectile;

    projectile.IsThisOnFire = 1;
    projectile.Damage = 9999;
    projectile.ParticlesPerSecond = 100;
    projectile.HowManyCooks = 50;
}

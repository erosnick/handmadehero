#include <Windows.h>
#include <cstdio>
#include <iostream>

using namespace std;

struct Projectile
{
	unsigned char IsThisOnFire;
	int Damage;
	int ParticlesPerSecond;
	short HowManyCooks;
};

struct SizeTest
{
	double One;
	double Two;
	char Three;
	char Four;
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

    size_t size = sizeof(Projectile);   // size == 16

	unsigned short* MrPointerMan = (unsigned short*)&projectile;

    Projectile projectiles[40];

    projectiles[30].Damage = 10;

    Projectile* projectilesPointer = projectiles;

    (projectilesPointer + 30)->Damage = 20;

    ((Projectile*)((char*)projectilesPointer + 30 * sizeof (Projectile)))->Damage = 30;

	SizeTest sizeTest;	

	cout << sizeof(SizeTest) << endl;	

	int x = 0xA;

	x = x << 1;
	x = x << 1;
	x = x << 1;
	
}

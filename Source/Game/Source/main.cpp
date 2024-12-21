#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE
#endif
#endif

#include "Game.h"

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
	return Kaka::Game{}.Go();
}
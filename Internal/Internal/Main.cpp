#include "HackFunctions.cpp"

using namespace hacks;

DWORD WINAPI hackthread(LPVOID param)
{
	// Enable a console for debugging purposes
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	GetHandles();

	while (true) {
		GetPlayerInfo();

		if (*player.base != 0 && *player.in_game == 6) {
			GetEntityListInfo();
			TriggerBot();
			ControlRecoil();
			GlowEsp();
		}

		Sleep(1);
	}

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		CreateThread(nullptr, 0, hackthread, nullptr, 0, nullptr);
		DisableThreadLibraryCalls(hModule);
		break;

	default:
		break;
	}
	return TRUE;
}
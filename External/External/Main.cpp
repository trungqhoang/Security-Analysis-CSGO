#include "HackFunctions.cpp"

using namespace hacks;

int main() {
	int in_game;
	while (true) {
		GetPlayerInfo();
		GetEntityListInfo();

		in_game = memory.Read<int>(player.client_state + 0x100);
		if (in_game == 6) {
			TriggerBot();
			ControlRecoil();
			Aimbot();
			GlowEsp();
		}

		Sleep(5);
	}
}
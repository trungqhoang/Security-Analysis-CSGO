#include "Memory.h"

Memory::Memory() {
	hprocess.OpenHandle("Counter-Strike: Global Offensive", "csgo.exe");
}

Memory::~Memory() {
	CloseHandle(hprocess.GetHandle());
}

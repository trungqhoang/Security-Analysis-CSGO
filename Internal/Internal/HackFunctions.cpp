#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <Windows.h>
#include <cmath>

#include "LocalPlayer.h"
#include "EntityList.h"
#include "GlowObject.h"

#include "Offsets.h"

using namespace std;

/**
 * Contains a set of functions used to exploit game data.
 */

namespace hacks {
	// Base addresses used across multiple functions
	static DWORD dwClientBase;
	static DWORD *dwClientState;
	static DWORD dwEngineBase;

	// Structs of objects to influence
	static LocalPlayer player;
	static EntityList entity_list[64];
	static GlowObject glow_object_list[128];

	// Player view angles
	static float old_punch[2] = { 0.0f, 0.0f };
	static float aim_angles[3] = { 0.0f, 0.0f, 0.0f };

	/// Acquires a handle to "client.dll" and "engine.dll" game files.
	/// These are used to access specific information in regards to entities and the player
	static void GetHandles() {
		while (!dwClientBase) dwClientBase = (DWORD)GetModuleHandle("client.dll");
		while (!dwEngineBase) dwEngineBase = (DWORD)GetModuleHandle("engine.dll");
	}

	/// Read from memory for necessary player information
	static void GetPlayerInfo() {
		player.base = (DWORD*)(dwClientBase + m_dwLocalPlayer);
		if (player.base == 0) return;

		player.team = (int*)(*player.base + m_iTeamNum);
		player.crosshair_id = (int*)(*player.base + m_iCrosshairID);
		player.shots_fired = (int*)(*player.base + m_iShotsFired);

		player.punch[0] = (float*)(*player.base + m_vecPunch);
		player.punch[1] = (float*)(*player.base + m_vecPunch + 0x4);

		player.position[0] = (float*)(*player.base + m_vecOrigin);
		player.position[1] = (float*)(*player.base + m_vecOrigin + 0x4);
		player.position[2] = (float*)(*player.base + m_vecOrigin + 0x8);

		dwClientState = (DWORD*)(dwEngineBase + m_dwClientState);
		player.in_game = (int*)(*dwClientState + 0x100);
		player.local_index = (int*)(*dwClientState + m_dwLocalPlayerIndex);
		player.view_angles[0] = (float*)(*dwClientState + m_dwViewAngles);
		player.view_angles[1] = (float*)(*dwClientState + m_dwViewAngles + 0x4);
		player.view_angles[2] = (float*)(*dwClientState + m_dwViewAngles + 0x8);
	}

	/// Read from memory for necessary entity information, for all (possible) entities
	static void GetEntityListInfo() {
		for (int i = 0; i < 64; i++) {
			entity_list[i].base = (DWORD*)(dwClientBase + m_dwEntityList + (i * m_dwEntityLoopDist));
			if (entity_list[i].base == 0) continue;

			entity_list[i].team = (int*)(*entity_list[i].base + m_iTeamNum);
			entity_list[i].health = (int*)(*entity_list[i].base + m_iHealth);
			entity_list[i].spotted_mask = (int*)(*entity_list[i].base + m_bSpottedByMask);
			entity_list[i].dormant = (bool*)(*entity_list[i].base + m_bDormant);
			entity_list[i].glow_index = (int*)(*entity_list[i].base + m_iGlowIndex);

			entity_list[i].bone_base = (DWORD*)(*entity_list[i].base + m_dwBoneMatrix);
			entity_list[i].position[0] = (float*)(*entity_list[i].bone_base + 8 * 0x30 + 0x0C);
			entity_list[i].position[1] = (float*)(*entity_list[i].bone_base + 8 * 0x30 + 0x1C);
			entity_list[i].position[2] = (float*)(*entity_list[i].bone_base + 8 * 0x30 + 0x2C);
		}
	}

	/// Automatically shoot if the player has a valid enemy target selected
	static void TriggerBot() {
		// Sanity checks on pointers
		if (GetAsyncKeyState('J') == 0 || player.crosshair_id == 0) return;
		if (player.team == 0) return;
		if (entity_list[*player.crosshair_id - 1].team == 0 || entity_list[*player.crosshair_id - 1].health == 0)
			return;

		// If target team is not the player's team, simulate a mouse click to shoot
		if (*entity_list[*player.crosshair_id - 1].team != *player.team &&
			*entity_list[*player.crosshair_id - 1].health > 0) {
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			Sleep(100);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(75);
		}
	}

	/// Using recoil information read from memory, adjust the player's view angle to compensate
	static void ControlRecoil() {
		// Do nothing if the player has fired less than one bullet
		if (*player.shots_fired <= 1) {
			old_punch[0] = *player.punch[0];
			old_punch[1] = *player.punch[1];
			return;
		}

		// Adjust player view angle to compensate for punch
		if (*player.punch[0] != 0 || *player.punch[1] != 0) {
			*player.view_angles[0] = *player.view_angles[0] - (*player.punch[0] * 2.0f) + (old_punch[0] * 2.0f);
			*player.view_angles[1] = *player.view_angles[1] - (*player.punch[1] * 2.0f) + (old_punch[1] * 2.0f);
		}

		old_punch[0] = *player.punch[0];
		old_punch[1] = *player.punch[1];
	}

	/// Force objects to glow, thus revealing enemies behind walls and other objects
	static void GlowEsp() {
		// Acquire the base for the glow object list
		DWORD* glow_base = (DWORD*)(dwClientBase + m_dwGlowObject);

		// For each (possible) entity in the game,
		for (int i = 0; i < 64; i++) {
			// If either the player.base or entity_list[i].base is missing, do nothing
			if (!entity_list[i].base || !player.base) continue;
			// nullptr checks
			if (entity_list[i].dormant || entity_list[i].health == 0 || entity_list[i].glow_index == 0) continue;

			// Access the data at the desired glow object
			GlowObject* glow_object = (GlowObject*)(*glow_base + (*entity_list[i].glow_index * sizeof(GlowObject)));
			float* glowA = (float*)(*glow_base + (*entity_list[i].glow_index * 0x38) + 0x10);
			float* glowR = (float*)(*glow_base + (*entity_list[i].glow_index * 0x38) + 0x4);
			float* glowG = (float*)(*glow_base + (*entity_list[i].glow_index * 0x38) + 0x8);
			float* glowB = (float*)(*glow_base + (*entity_list[i].glow_index * 0x38) + 0xC);
			BOOL* glowO = (BOOL*)(*glow_base + (*entity_list[i].glow_index * 0x38) + 0x24);
			BOOL* glowU = (BOOL*)(*glow_base + (*entity_list[i].glow_index * 0x38) + 0x25);

			// Modify glow object value based on team number.
			if (*entity_list[i].team == 2) {
				*glowA = 1.0f;
				*glowR = 0.7f;
				*glowG = 0.4f;
				*glowB = 0.1f;
				*glowO = true;
				*glowU = false;
			}
			else if (*entity_list[i].team == 3) {
				*glowA = 1.0f;
				*glowR = 0.1f;
				*glowG = 0.4f;
				*glowB = 0.7f;
				*glowO = true;
				*glowU = false;
			}
		}
	}
}
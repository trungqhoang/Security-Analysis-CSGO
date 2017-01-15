#include <iostream>
#include <cmath>

#include "LocalPlayer.h"
#include "EntityList.h"
#include "GlowObject.h"

#include "Memory.h"
#include "Offsets.h"

using namespace std;

namespace hacks {
	// Wrapper object used to read and write to physical memory
	static Memory memory;

	// Structs of objects to influence
	static LocalPlayer player;
	static EntityList entity_list[64];
	static GlowObject glow_object_list[128];

	// Player view angles
	static float old_punch[2] = { 0.0f, 0.0f };
	static float aim_angles[3] = { 0.0f, 0.0f, 0.0f };

	/// Read from memory for necessary player information
	static void GetPlayerInfo() {
		player.base = memory.Read<DWORD>(memory.hprocess.GetClient() + m_dwLocalPlayer);
		player.team = memory.Read<int>(player.base + m_iTeamNum);
		player.crosshair_id = memory.Read<int>(player.base + m_iCrosshairID);
		player.shots_fired = memory.Read<int>(player.base + m_iShotsFired);

		player.punch[0] = memory.Read<float>(player.base + m_vecPunch);
		player.punch[1] = memory.Read<float>(player.base + m_vecPunch + 0x4);

		player.position[0] = memory.Read<float>(player.base + m_vecOrigin);
		player.position[1] = memory.Read<float>(player.base + m_vecOrigin + 0x4);
		player.position[2] = memory.Read<float>(player.base + m_vecOrigin + 0x8);

		player.client_state = memory.Read<DWORD>(memory.hprocess.GetEngine() + m_dwClientState);
		player.local_index = memory.Read<int>(player.client_state + m_dwLocalPlayerIndex);
		player.view_angles[0] = memory.Read<float>(player.client_state + m_dwViewAngles);
		player.view_angles[1] = memory.Read<float>(player.client_state + m_dwViewAngles + 0x4);
		player.view_angles[2] = memory.Read<float>(player.client_state + m_dwViewAngles + 0x8);
	}

	/// Read from memory for necessary entity information, for all (possible) entities
	static void GetEntityListInfo() {
		for (int i = 0; i < 64; i++) {
			entity_list[i].base = memory.Read<DWORD>(memory.hprocess.GetClient() + m_dwEntityList +
				(i * m_dwEntityLoopDist));
			entity_list[i].team = memory.Read<int>(entity_list[i].base + m_iTeamNum);
			entity_list[i].health = memory.Read<int>(entity_list[i].base + m_iHealth);
			entity_list[i].spotted_mask = memory.Read<int>(entity_list[i].base + m_bSpottedByMask);
			entity_list[i].dormant = memory.Read<bool>(entity_list[i].base + m_bDormant);
			entity_list[i].glow_index = memory.Read<int>(entity_list[i].base + m_iGlowIndex);

			entity_list[i].bone_base = memory.Read<DWORD>(entity_list[i].base + m_dwBoneMatrix);
			entity_list[i].position[0] = memory.Read<float>(entity_list[i].bone_base + 8 * 0x30 + 0x0C);
			entity_list[i].position[1] = memory.Read<float>(entity_list[i].bone_base + 8 * 0x30 + 0x1C);
			entity_list[i].position[2] = memory.Read<float>(entity_list[i].bone_base + 8 * 0x30 + 0x2C);
		}
	}

	/// Automatically shoot if the player has a valid enemy target selected
	static void TriggerBot() {
		if (GetAsyncKeyState('J') == 0 || player.crosshair_id == 0) return;

		if (entity_list[player.crosshair_id - 1].team != player.team &&
			entity_list[player.crosshair_id - 1].health > 0) {
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			Sleep(100);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(75);
		}
	}

	/// Using recoil information read from memory, adjust the player's view angle to compensate
	static void ControlRecoil() {
		// Do nothing if the player has fired less than one bullet
		if (player.shots_fired <= 1) {
			old_punch[0] = player.punch[0];
			old_punch[1] = player.punch[1];
			return;
		}

		// Adjust player view angle to compensate for punch
		if (player.punch[0] != 0 || player.punch[1] != 0) {
			player.view_angles[0] -= player.punch[0] * 2.0f;
			player.view_angles[1] -= player.punch[1] * 2.0f;
			player.view_angles[0] += old_punch[0] * 2.0f;
			player.view_angles[1] += old_punch[1] * 2.0f;

			// Adjust for valid values
			if (player.view_angles[0] >= 89) player.view_angles[0] = 89;
			if (player.view_angles[0] <= -89) player.view_angles[0] = -89;
			if (player.view_angles[1] <= -180) player.view_angles[1] += 360;
			if (player.view_angles[1] >= 180) player.view_angles[1] -= 360;

			memory.Write<float>(player.client_state + m_dwViewAngles, player.view_angles[0]);
			memory.Write<float>(player.client_state + m_dwViewAngles + 0x4, player.view_angles[1]);
		}

		old_punch[0] = player.punch[0];
		old_punch[1] = player.punch[1];
	}

	/// Force objects to glow, thus revealing enemies behind walls and other objects
	static void GlowEsp() {
		memory.read_fail_count = 0;
		// Acquire the base for the glow object list
		DWORD glow_base = memory.Read<DWORD>(memory.hprocess.GetClient() + m_dwGlowObject);

		// If we memory wasn't read properly, return
		if (memory.read_fail_count > 0) {
			memory.read_fail_count = 0;
			return;
		}

		// For each (possible) entity in the game,
		for (int i = 0; i < 64; i++) {
			// Ensure that the object is a valid object
			if (entity_list[i].dormant == 1 || entity_list[i].team == 0) continue;
			if (!(entity_list[i].spotted_mask & (int)pow((double)2, (double)player.local_index)))
				continue;

			// Modify glow object value based on team number.
			if (entity_list[i].team == 2) {
				memory.Write<float>(glow_base + (entity_list[i].glow_index * 0x38) + 0x10, 1.0f);
				memory.Write<float>(glow_base + (entity_list[i].glow_index * 0x38) + 0x4, 0.7f);
				memory.Write<float>(glow_base + (entity_list[i].glow_index * 0x38) + 0x8, 0.4f);
				memory.Write<float>(glow_base + (entity_list[i].glow_index * 0x38) + 0xC, 0.1f);
				memory.Write<BOOL>(glow_base + (entity_list[i].glow_index * 0x38) + 0x24, true);
				memory.Write<BOOL>(glow_base + (entity_list[i].glow_index * 0x38) + 0x25, false);
			}
			else if (entity_list[i].team == 3) {
				memory.Write<float>(glow_base + (entity_list[i].glow_index * 0x38) + 0x10, 1.0f);
				memory.Write<float>(glow_base + (entity_list[i].glow_index * 0x38) + 0x4, 0.1f);
				memory.Write<float>(glow_base + (entity_list[i].glow_index * 0x38) + 0x8, 0.4f);
				memory.Write<float>(glow_base + (entity_list[i].glow_index * 0x38) + 0xC, 0.7f);
				memory.Write<BOOL>(glow_base + (entity_list[i].glow_index * 0x38) + 0x24, true);
				memory.Write<BOOL>(glow_base + (entity_list[i].glow_index * 0x38) + 0x25, false);
			}
		}
	}

	/// Given the position of two entities, calculate the required angles to aim from the
	/// source to the destination.
	static void CalcAngle(float* src, float* dst, float* angle) {
		double delta[3] = { (src[0] - dst[0]), (src[1] - dst[1]), (src[2] - dst[2]) };
		double hyp = sqrt(delta[0] * delta[0] + delta[1] * delta[1]);
		angle[0] = (float)(asinf(delta[2] / hyp) * 57.295779513082f);
		angle[1] = (float)(atanf(delta[1] / delta[0]) * 57.295779513082f);
		angle[2] = 0;

		if (delta[0] >= 0.0f) angle[1] += 180.0f;
		if (angle[1] >= 180) angle[1] -= 360.0f;
	}

	/// Adjust the player's view angles to point at a target.
	/// Use a gradual motion to simulate human movmement
	static void Aimbot() {
		// Sanity checks on player target
		if (player.crosshair_id > 0 && player.crosshair_id <= 64) {
			player.target = player.crosshair_id - 1;
		}

		if (entity_list[player.target].team == player.team || entity_list[player.target].health <= 0) return;
		if (!(entity_list[player.target].spotted_mask & (int)pow((double)2, (double)player.local_index))) {
			player.target = 0;
			return;
		}

		if (GetAsyncKeyState('J') != 0) return;
		if (player.shots_fired <= 1) return;

		CalcAngle(player.position, entity_list[player.target].position, aim_angles);
		aim_angles[1] -= old_punch[1] * 2.0f;

		// Adjust player view angles to match required angles
		if (aim_angles[1] - 1 > player.view_angles[1]) {
			player.view_angles[1] += 0.5;
			if (player.view_angles[1] <= -180) player.view_angles[1] += 360;
			if (player.view_angles[1] >= 180) player.view_angles[1] -= 360;
			memory.Write<float>(player.client_state + m_dwViewAngles, player.view_angles[0]);
			memory.Write<float>(player.client_state + m_dwViewAngles + 0x4, player.view_angles[1]);
		}
		else if (aim_angles[1] + 1 < player.view_angles[1]) {
			player.view_angles[1] -= 0.5;
			if (player.view_angles[1] <= -180) player.view_angles[1] += 360;
			if (player.view_angles[1] >= 180) player.view_angles[1] -= 360;
			memory.Write<float>(player.client_state + m_dwViewAngles, player.view_angles[0]);
			memory.Write<float>(player.client_state + m_dwViewAngles + 0x4, player.view_angles[1]);
		}
		else {
			player.view_angles[1] = aim_angles[1];
			memory.Write<float>(player.client_state + m_dwViewAngles, player.view_angles[0]);
			memory.Write<float>(player.client_state + m_dwViewAngles + 0x4, player.view_angles[1]);
		}
	}
}
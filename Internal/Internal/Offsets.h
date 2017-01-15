#pragma once

#include <Windows.h>

/**
 * A list of all offsets used to access specific locations in memory. (Outdated)
 */

namespace {
	const DWORD m_dwLocalPlayer = 0x00AA581C;			// Base address of the player
	const DWORD m_dwGlowObject = 0x04FE2784;			// Base address of the glow list
	const DWORD m_dwClientState = 0x005C7594;			// Based address of the client state
	const DWORD m_dwEntityList = 0x04AC8034;			// Base address of the entity list
	const DWORD m_dwEntityLoopDist = 0x10;				// Distance in memory each entity is from one another

	const DWORD m_vecPunch = 0x301C;				// Offset for recoil
	const DWORD m_iTeamNum = 0xF0;					// Offset for team number
	const DWORD m_iHealth = 0xFC;					// Offset for health
	const DWORD m_iCrosshairID = 0xAA70;				// Offset for target ID
	const DWORD m_iHAngle = 0x12C;					// Offset for the horizontal angle of the player
	const DWORD m_iShotsFired = 0xA2C0;				// Offset for number of bullets fired
	const DWORD m_vecOrigin = 0x134;				// Offset for entity position
	const DWORD m_dwBoneMatrix = 0x2698;				// Offset for the bone matrix
	const DWORD m_bSpottedByMask = 0x97C;				// Offset for targets visible entity is visible to
	const DWORD m_dwViewAngles = 0x4D0C;				// Offset for current view angles
	const DWORD m_iGlowIndex = 0xA320;				// Offset for index in the list of glowable objects
	const DWORD m_bDormant = 0xE9;					// Offset for dormant status
	const DWORD m_dwLocalPlayerIndex = 0x178;			// Offset for index of player in entity list
}

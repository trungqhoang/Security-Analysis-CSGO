#pragma once

#include <Windows.h>

/**
* A struct containing important information in regards to the player.
*/

struct LocalPlayer {
	DWORD base;			// Base address in memory of the player
	int team;			// The team number the player
	int crosshair_id;		// The player's current target
	int target;			// The player's current target
	int shots_fired;		// The number of bullets fired by the player
	float punch[2];			// The amount of recoil applied to the player
	float position[3];		// The position of the player

	DWORD client_state;		// Base address of the client state
	int local_index;		// Index of the player with respect to the entity list
	float view_angles[3];		// The current view angles of the player
};

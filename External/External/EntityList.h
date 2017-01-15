#pragma once

#include <Windows.h>

/**
* A struct containing important information in regards to entities in a game server.
*/

struct EntityList {
	DWORD base;				// Base address of this entity
	int team;				// The team of this entity
	int health;				// The current health of this entity
	int spotted_mask;		// A 32 bit number representing other entities this entity is visible to
	bool dormant;			// Whether or not this entity is dormant
	int glow_index;			// The index of this entity in a list of objects that can glow

	DWORD bone_base;		// Base address of this entity's bone structure list
	float position[3];		// The position of this entity's target bone
};
#pragma once

/**
* A struct containing important information in regards to the glowable objects.
*/

struct GlowObject {
	void* pEntity;				// The entity to glow
	float* r;				// 0-255 RED glow coloration
	float* g;				// 0-255 GREEN glow coloration
	float* b;				// 0-255 BLUE glow coloration
	float* a;				// 0-1 ALPHA glow opacity
	unsigned char* unk0[0x10];		// Fluff data
	bool* m_bRenderWhenOccluded;		// Whether or not to make the object glow occluded
	bool* m_bRenderWhenUnoccluded;		// Whether or not to make the object glow unoccluded
	unsigned char*  unk1[0x12];		// Fluff data
};

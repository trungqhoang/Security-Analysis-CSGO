#pragma once

#include <Windows.h>

#include "HProcess.h"

class Memory {

public:
	HProcess hprocess;
	int read_fail_count = 0;

	Memory();

	// Read a location in memory
	template <typename type>
	type Read(DWORD read_addr) {
		type buffer;
		if (ReadProcessMemory(hprocess.GetHandle(), (PBYTE*)read_addr, &buffer, sizeof(type), NULL) == 0)
			read_fail_count++;
		return buffer;
	}

	// Write to a location in memory
	template <typename type>
	BOOL Write(DWORD write_addr, type write_val) {
		return WriteProcessMemory(hprocess.GetHandle(), (PBYTE*)write_addr, &write_val, sizeof(type), NULL);
	}

	~Memory();

};
#include "HProcess.h"

bool HProcess::OpenHandle(char* window_name, char* process_name) {
	try {
		SetDebugPrivileges();

		// Look through all running processes to find one matching process_name
		// Do so once every second
		while (!FindProcess(process_name)) Sleep(1000);
		handle = OpenProcess(PROCESS_ALL_ACCESS, false, game_process.th32ProcessID);

		// Loop through all of the process' modules to find client and engine
		client = 0x0;
		engine = 0x0;

		while (client == 0x0) client = GetModule("client.dll", game_process.th32ProcessID);
		while (engine == 0x0) engine = GetModule("engine.dll", game_process.th32ProcessID);

		// Acquire a handle to the specified window
		hwnd = FindWindow(NULL, window_name);

		// Return true on success
		return true;
	}
	catch (...) {
		// If any exception occured, return false
		return false;
	}
}

HANDLE HProcess::GetHandle() {
	return HProcess::handle;
}

DWORD HProcess::GetClient() {
	return HProcess::client;
}

DWORD HProcess::GetEngine() {
	return HProcess::engine;
}

void HProcess::SetDebugPrivileges() {
	HANDLE handle = GetCurrentProcess(), token;
	TOKEN_PRIVILEGES priv;
	LUID luid;

	OpenProcessToken(handle, TOKEN_ADJUST_PRIVILEGES, &token);
	LookupPrivilegeValue(0, "seDebugPrivilege", &luid);

	priv.PrivilegeCount = 1;
	priv.Privileges[0].Luid = luid;
	priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(token, false, &priv, 0, 0, 0);

	CloseHandle(token);
	CloseHandle(handle);
}

bool HProcess::FindProcess(char* process_name) {
	PROCESSENTRY32 process_entry;
	process_entry.dwSize = sizeof(PROCESSENTRY32);

	// Instantiate a handle to the first process found
	HANDLE snapshot;
	snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	// While there are unchecked processes, place the next snapshot into process_entry
	while (Process32Next(snapshot, &process_entry)) {
		// If a process with the specified name has been found,
		if (_strcmpi(process_entry.szExeFile, process_name) == 0) {
			// Copy the found process entry into game_proces
			memcpy(&game_process, &process_entry, sizeof(PROCESSENTRY32));
			CloseHandle(snapshot);

			return true;
		}
	}

	// If the specified process name was not found, close handle and return false
	CloseHandle(snapshot);
	return false;
}

DWORD HProcess::GetModule(LPSTR module_name, DWORD process_id) {
	MODULEENTRY32 module_entry;
	module_entry.dwSize = sizeof(module_entry);

	// Instantiate a handle to the first module found
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id);

	// While there are unchecked modules, place the next snapshot into module_entry
	while (Module32Next(snapshot, &module_entry)) {
		// If a module with the specified name has been found,
		if (!strcmp(module_entry.szModule, module_name)) {
			// Close the handle and return the vaddr of 
			CloseHandle(snapshot);
			return (DWORD)module_entry.modBaseAddr;
		}
	}

	// Close the handle and return NULL if nothing is found
	CloseHandle(snapshot);
	return NULL;
}
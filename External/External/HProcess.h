#ifndef HPROCESS_H_
#define HPROCESS_H_

#include <Windows.h>
#include <TlHelp32.h>

class HProcess {
public:
	// Opens a handle to the specified process name for memory access
	bool OpenHandle(char* window_name, char* process_name);

	// Returns a handle to the process
	HANDLE GetHandle();

	// Returns the 32-bit (4 bytes) address of "client.dll"
	DWORD GetClient();

	// Returns the 32-bit (4 bytes) address of "engine.dll"
	DWORD GetEngine();

private:
	PROCESSENTRY32 game_process;
	HANDLE handle;
	HWND hwnd;
	DWORD client, engine;

	// Give this process debug privileges
	void SetDebugPrivileges();

	// Acquire the targetted process. Returns TRUE on success, FALSE otherwise
	bool FindProcess(char* process_name);

	// Acquire the specified module of the process
	DWORD GetModule(LPSTR module_name, DWORD process_id);

};
#endif
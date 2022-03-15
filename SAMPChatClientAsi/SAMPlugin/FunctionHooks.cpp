#include "../main.h"


Hook::Hook(void* Dest, void* Patch, int HookLength)
{
	this->Dest = (DWORD)Dest;
	this->TrampolineLength = HookLength + 5;
	this->Trampoline = (DWORD)VirtualAlloc(0, HookLength+5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	memcpy((void*)this->Trampoline, Dest, HookLength);

	DWORD JmpAddress = this->Trampoline + HookLength;
	DWORD DestAndTrampolineOffset = (((DWORD)Dest + HookLength) - JmpAddress) - 5;

	*(BYTE*)JmpAddress = 0xE9; //jmp
	*(DWORD*)(JmpAddress+1) = DestAndTrampolineOffset;

	DWORD protection;
	VirtualProtect(Dest, HookLength, PAGE_READWRITE, &protection);

	DestAndTrampolineOffset = ((DWORD)Patch - (DWORD)Dest) - 5;

	*(BYTE*)Dest = 0xE9;
	*(DWORD*)((DWORD)Dest + 1) = DestAndTrampolineOffset;

	DWORD protectionTemp;
	VirtualProtect(Dest, HookLength, protection, &protectionTemp);
}

Hook::~Hook()
{
	DWORD protection;
	VirtualProtect((void*)this->Dest, this->TrampolineLength - 5, PAGE_READWRITE, &protection);

	memcpy((void*)this->Dest, (void*)this->Trampoline, this->TrampolineLength - 5);

	DWORD protectionTemp;
	VirtualProtect((void*)this->Dest, this->TrampolineLength - 5, protection, &protectionTemp);
	//
	VirtualFree((void*)this->Trampoline, 0, MEM_RELEASE);
}

DWORD Hook::GetFunctionAddress()
{
	return this->Trampoline;
}
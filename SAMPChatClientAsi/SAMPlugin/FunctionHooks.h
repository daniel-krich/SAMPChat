

class Hook {
private:
	DWORD Trampoline;
	DWORD Dest;
	DWORD TrampolineLength;
public:
	DWORD GetFunctionAddress();
	Hook(void* Dest, void* Patch, int HookLength);
	~Hook();
};

//------------	Some reference on how to call the "original" function
// 
// ((int(_stdcall*)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType))(test->GetFunctionAddress()))(hWnd, lpText, lpCaption, uType);
//
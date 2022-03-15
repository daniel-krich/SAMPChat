#include "main.h"

HWND hWndEdit;

std::list<std::string>ConsoleData;

INT WINAPI WinMain(_In_ HINSTANCE CurrentInstance, _In_opt_ HINSTANCE PreviousInstance, _In_ PSTR CmdLine, _In_ INT CmdCount)
{
#ifndef _NDEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_WNDW);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
	//_CrtSetBreakAlloc(79);
#endif
	const wchar_t *ClassName = L"SAMPChat";
	WNDCLASS wd = {};
	wd.hInstance = CurrentInstance;
	wd.lpszClassName = ClassName;
	wd.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wd.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wd.lpfnWndProc = WindowActions;
	RegisterClass(&wd);
	HWND hWnd = CreateWindow(ClassName, ClassName, WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, nullptr, nullptr);
	hWndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), TEXT(""),
		WS_CHILD | WS_VISIBLE | WS_VSCROLL |
		ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 0, 0, 805,
		560, hWnd, NULL, NULL, NULL);
	CreateThread(NULL, 0, ServerThread, NULL, 0, NULL);
	MSG msg = {};
	while (GetMessage(&msg, nullptr, NULL, NULL))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

LRESULT CALLBACK WindowActions(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{	
	switch (msg)
	{
		case WM_DESTROY:
		{
			Server->Exit();
			return 0;
		}
		case WM_CREATE:
		{
			return 0;
		}
		default:
		{
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
	}
}

void PrintGUI(std::string lpString)
{
	time_t _time = time(0);
	struct tm* __time = new tm;
	localtime_s(__time , &_time);
	//
	std::string timeparse('[' + std::to_string(__time->tm_hour) + ':' + std::to_string(__time->tm_min) + ':' + std::to_string(__time->tm_sec) + "] ");
	std::string Temp;
	delete __time;
	//
	std::replace_if(lpString.begin(), lpString.end(), [](char repl) {
		if (repl == '\r' || repl == '\n') return true;
		else return false;
	}, ' ');
	lpString = timeparse + lpString;
	if (ConsoleData.size() >= PrintGUI_MAX_LINES)
	{
		ConsoleData.pop_front();
		ConsoleData.push_back(lpString.substr(0, 93) + (lpString.length() >= 92 ? "..." : ""));
	}
	else
	{
		ConsoleData.push_back(lpString.substr(0, 93) + (lpString.length() >= 92 ? "..." : ""));
	}
	//
	GlueConsoleData(ConsoleData, Temp);
	SetWindowTextA(hWndEdit, Temp.c_str());
	SendMessage(hWndEdit, EM_LINESCROLL, 0, PrintGUI_MAX_LINES);
}

void GlueConsoleData(std::list<std::string> data, std::string& out)
{
	out.clear();
	for (std::list<std::string>::iterator iter = data.begin(); iter != data.end(); ++iter)
	{
		if (std::next(iter) != data.end())
		{
			out.append(iter->c_str());
			out.append("\r\n");
		}
		else out.append(iter->c_str());
	}
}
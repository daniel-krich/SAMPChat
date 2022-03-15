#include "main.h"

HINSTANCE				g_hOrigDll = NULL;
HMODULE					g_hDllModule = NULL;
uint32_t				g_dwSAMP_Addr = NULL;
D3DPRESENT_PARAMETERS*	g_pGTAPresent = (D3DPRESENT_PARAMETERS*)0xC9C040;

TCP::TCPClient			*client = nullptr;

DWORD WINAPI Entry(LPVOID params)
{
	waitForSamp();
	addToChatWindow("SAMPChat loaded", CHAT_RGB(153, 204, 255));
	client = new TCP::TCPClient("127.0.0.1", 2230);
	//client = new TCP::TCPClient("193.124.115.45", 2230);
	client->HandleConnection(TCP::OnReceiveDataCall, TCP::OnConnectionClosedCall); //While loop till connection close
	delete client;
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			CreateThread(NULL, 0, Entry, NULL, 0, NULL);
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			//if (client != nullptr)
			//{
			//	delete client;
			//}
			break;
		}
	}

	return true;
}
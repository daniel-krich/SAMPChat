#include "../main.h"

TCPServer* Server;

DWORD WINAPI ServerThread(void* arg)
{
	Server = new TCPServer(SERVER_PORT);
	Server->Initialize(OnClientConnectHandle, OnClientSendMessageHandle, OnClientDisconnectHandle);
	delete Server;
	exit(0);
	return 0;
}



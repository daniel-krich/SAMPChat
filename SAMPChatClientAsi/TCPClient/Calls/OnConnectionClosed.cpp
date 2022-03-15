#include "../main.h"

void TCP::OnConnectionClosedCall()
{
	addToChatWindow("[*] Connection to {FFFFFF}SAMPChat {ff9980}was closed.", CHAT_RGB(255, 153, 128));
}

void TCP::OnConnectionFailCall()
{
	addToChatWindow("[*] Connection to {FFFFFF}SAMPChat {ff9980}failed.", CHAT_RGB(255, 153, 128));
}
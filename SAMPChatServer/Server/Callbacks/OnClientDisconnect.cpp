#include "../../main.h"

void OnClientDisconnect(ConnectedClient* client)
{
	if (client->storage.m_LoggedIn == true)
	{
		for (ConnectedClient* f_client : client->serverInstance->ConnectedUsers())
		{
			if (f_client->storage.t_TempUserId_Invite == client->storage.m_ID)
			{
				f_client->storage.t_TempUserId_Invite = 0;
				f_client->SendClientMessage(str::format("{0} отключился, предложение отозвано.", client->storage.m_Name), CHAT_RGB(255, 136, 77));
			}
		}
		PrintGUI(str::format("{0} ({1}) отключился от сервера", client->storage.m_Name, client->GetIP()));
	}
	else PrintGUI(str::format("{0} отключился от сервера", client->GetIP()));
}

void OnClientDisconnectHandle(ConnectedClient* client)
{
	try
	{
		OnClientDisconnect(client);
	}
	catch (const std::exception& ex)
	{
		PrintGUI(str::format("OnClientDisconnect crash ({0}):", client->GetIP()));
		PrintGUI(ex.what());
	}
}
#include "../../main.h"

bool OnClientConnect(ConnectedClient* client)
{
	int ipDuplicates = client->serverInstance->FindUsersIf([&](ConnectedClient* f_client) {
		if (f_client->GetIP() == client->GetIP()) return true;
		else return false;
	}).size();
	//
	if (ipDuplicates >= 2) return false;
	else
	{
		PrintGUI(str::format("Incoming connection to the server ({0})", client->GetIP()));
		client->SendClientMessage("Greetings, enter {00cc66}!auth {FFFFFF}to login/register.", CHAT_RGB(255, 255, 255));
		client->SendClientMessage("View all available commands - {00cc66}!help", CHAT_RGB(255, 255, 255));
		return true;
	}
}

bool OnClientConnectHandle(ConnectedClient* client)
{
	try
	{
		return OnClientConnect(client);
	}
	catch (const std::exception& ex)
	{
		PrintGUI(str::format("OnClientConnect crash ({0}):", client->GetIP()));
		PrintGUI(ex.what());
		return false;
	}
}
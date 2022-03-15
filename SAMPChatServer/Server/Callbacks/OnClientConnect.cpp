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
		PrintGUI(str::format("Входящее подключение к серверу ({0})", client->GetIP()));
		client->SendClientMessage("Здравствуй, введите {00cc66}!auth {FFFFFF}для авторизации/регистрации.", CHAT_RGB(255, 255, 255));
		client->SendClientMessage("Если Вам нужна помощь с командами - {00cc66}!help", CHAT_RGB(255, 255, 255));
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
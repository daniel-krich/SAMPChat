#include "../../main.h"

void LoadFamiles(TCPServer* server);

void OnServerInit(TCPServer* server, unsigned short port)
{
	server->sql = new MySQLConnector();
	PrintGUI(str::format("[RUN] Server started successfully on port {0}", port));
	//
	LoadFamiles(server);
}

void LoadFamiles(TCPServer* server)
{
	sql::ResultSet* res = server->SQLInstance()->get("SELECT * FROM families");
	if (res != nullptr)
	{
		while (res->next())
		{
			FamilyStruct temp = { res->getInt("ID"), res->getString("Name"), (DWORD)res->getInt64("Color")};
			server->storage.families.push_back(temp);
		}
		PrintGUI(str::format("[Families] {0} families were loaded.", server->storage.families.size()));
		delete res;
	}
	else
	{
		PrintGUI("[Error] Families loading failed.");
	}
}
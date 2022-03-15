#include "../../main.h"

void OnCommandResponse(ConnectedClient* client, std::string data)
{
	str::Breaker breaker = str::breakOnSpace(data);
	if (client->storage.m_LoggedIn == false && breaker.Cmd != "auth")
	{
		client->SendClientMessage("�� �� ������������, !auth.", CHAT_RGB(255, 136, 77));
		return;
	}
	switch (str::hash(breaker.Cmd.c_str()))
	{
		case str::hash("auth"):
		{
			if (client->storage.m_LoggedIn == false)
			{
				client->ShowClientDialog(1, DialogStyles::DIALOG_STYLE_INPUT, "{FFCC00}�����������",
					str::format("{FFFFFF}����� ���������� �� SAMPChat!\n\n{FFFFFF}IP �����: {FFCC00}{0}\n{FFFFFF}������� ��� ������ ��������:\n", client->GetIP()),
					"�����", "������");
			}
			else client->SendClientMessage(str::format("�� ��� ������������, {FFFFFF}{0}.", client->storage.m_Name), CHAT_RGB(0, 204, 102));
			break;
		}
		case str::hash("help"):
		{
			client->ShowClientDialog(0, DialogStyles::DIALOG_STYLE_MSGBOX, "{00cc70}������ �� ��������",
				"\n{FFFFFF}!auth\t\t{00cc70}�����������/�����������\n{FFFFFF}!g\t\t{00cc70}���������� ���\n{FFFFFF}!f\t\t{00cc70}��� �����\n{FFFFFF}!finvdetails\t{00cc70}�������� �����������\n{FFFFFF}!finv\t\t{00cc70}������� � �����\n{FFFFFF}!funinv\t\t{00cc70}��������� �� �����\n{FFFFFF}!fpromote\t{00cc70}��������/�������� ���� ����� (��� ����������)\n{FFFFFF}!family\t\t{00cc70}������� �����/���� �����",
				"�������");
			break;
		}
		case str::hash("g"):
		{
			if (breaker.Params.length() > 0)
			{
				for (ConnectedClient* f_client : client->serverInstance->ConnectedUsers())
				{
					if (f_client->storage.m_LoggedIn == true) f_client->SendClientMessage(str::format("[*] {0}: {FFFFFF}{1}", client->storage.m_Name, breaker.Params), CHAT_RGB(255, 136, 77));
					else f_client->SendClientMessage("[*] ���� ����������� ���-�� ���������, ��� ��������� �������������.", CHAT_RGB(255, 136, 77));
				}
			}
			else client->SendClientMessage("���������: !g [�����]", CHAT_RGB(255, 255, 255));
			break;
		}
		case str::hash("f"):
		{
			if (client->storage.m_FamilyID != 0)
			{
				if (breaker.Params.length() > 0)
				{
					std::list<FamilyStruct>::iterator famIt = std::find_if(client->serverInstance->storage.families.begin(), client->serverInstance->storage.families.end(), [&](FamilyStruct& f_Fam) {
						if (f_Fam.id == client->storage.m_FamilyID) return true;
						else return false;
					});
					for (ConnectedClient* f_client : client->serverInstance->ConnectedUsers())
					{
						if (f_client->storage.m_FamilyID == client->storage.m_FamilyID)
						{
							f_client->SendClientMessage(str::format("[{0}] {d9d9d9}{1}: {FFFFFF}{2}", famIt->name, client->storage.m_Name, breaker.Params), famIt->color);
						}
					}
				}
				else client->SendClientMessage("���������: !f [�����]", CHAT_RGB(255, 255, 255));
			}
			else client->SendClientMessage("�� �� �������� � �����", CHAT_RGB(255, 255, 255));
			break;
		}
		case str::hash("finvdetails"):
		{
			if (client->storage.m_FamilyID == 0 && client->storage.t_TempUserId_Invite != 0)
			{
				std::list<ConnectedClient*> f_clients = client->serverInstance->FindUsersIf([&](ConnectedClient* f_client) {
					if (f_client->storage.m_ID == client->storage.t_TempUserId_Invite) return true;
					else return false;
				});
				if (f_clients.size() > 0)
				{
					ConnectedClient* f_client = f_clients.back();
					if (f_client->storage.m_FamilyID != 0)
					{
						std::list<FamilyStruct>::iterator famIt = std::find_if(client->serverInstance->storage.families.begin(), client->serverInstance->storage.families.end(), [&](FamilyStruct& f_Fam) {
							if (f_Fam.id == f_client->storage.m_FamilyID) return true;
							else return false;
						});
						client->ShowClientDialog(9, DialogStyles::DIALOG_STYLE_MSGBOX, str::format("{{0}}�������� �����������", str::NumToHex(famIt->color)),
							str::format("{FFFFFF}���������:\t\t{{0}}{1}\n{FFFFFF}�����:\t\t\t{{2}}{3}\n", str::NumToHex(famIt->color), f_client->storage.m_Name, str::NumToHex(famIt->color), famIt->name), "�������", "��������");
					}
					else
					{
						client->SendClientMessage("����������� ��� �� ���������.", CHAT_RGB(255, 136, 77));
						client->storage.t_TempUserId_Invite = 0;
					}
				}
				else
				{
					client->SendClientMessage("����������� ��� �� ���������.", CHAT_RGB(255, 136, 77));
					client->storage.t_TempUserId_Invite = 0;
				}
			}
			else client->SendClientMessage("��� ����� �����������.", CHAT_RGB(255, 136, 77));
			break;
		}
		case str::hash("finv"):
		{
			if (client->storage.m_FamilyID != 0 && client->storage.m_FamilyRank >= 1)
			{
				if (breaker.Params.length() >= 4 && breaker.Params.length() <= 25)
				{
					std::list<ConnectedClient*> f_clients = client->serverInstance->FindUsersIf([&](ConnectedClient* f_client) {
						if (f_client->storage.m_LoggedIn == true && str::caseInSensStringCompare(f_client->storage.m_Name, breaker.Params)) return true;
						else return false;
					});
					if (f_clients.size() > 0)
					{
						ConnectedClient* f_client = f_clients.back();
						if (f_client->storage.m_LoggedIn == true)
						{
							if (f_client->storage.m_FamilyID == 0)
							{
								if (f_client->storage.t_TempUserId_Invite == 0)
								{
									client->SendClientMessage(str::format("�� ���������� {00CC70}{0} {FFFFFF}�������� � ���� �����.", f_client->storage.m_Name), CHAT_RGB(255, 255, 255));
									//
									f_client->storage.t_TempUserId_Invite = client->storage.m_ID;
									f_client->SendClientMessage(str::format("��������� ����� ����������� � ����� �� {00CC70}{0}.", client->storage.m_Name), CHAT_RGB(255, 255, 255));
									f_client->SendClientMessage("��� ��������� ����������� ������� {00CC70}!finvdetails", CHAT_RGB(255, 255, 255));
								}
								else if (f_client->storage.t_TempUserId_Invite == client->storage.m_ID)
								{
									client->SendClientMessage(str::format("�� �������� ����������� � {0}.", f_client->storage.m_Name), CHAT_RGB(255, 136, 77));
									//
									f_client->storage.t_TempUserId_Invite = 0;
									f_client->SendClientMessage(str::format("{0} ������� �����������.", client->storage.m_Name), CHAT_RGB(255, 136, 77));
								}
								else client->SendClientMessage("������� ������������ ��� ���-�� ��������� �������� � �����, ��������� ���� ������.", CHAT_RGB(255, 255, 255));
							}
							else client->SendClientMessage("������ ������������ ��� ������� � �����.", CHAT_RGB(255, 255, 255));
						}
						else client->SendClientMessage("������ ������������ �� �����������.", CHAT_RGB(255, 255, 255));
					}
					else client->SendClientMessage("������ ������������ �� ������ � ����.", CHAT_RGB(255, 255, 255));
				}
				else client->SendClientMessage("���������: !finv [���], ��� ������ �������� �� 4-25 ��������", CHAT_RGB(255, 255, 255));
			}
			else client->SendClientMessage("�� �� �������� � �����, ���� �� ���������/���.", CHAT_RGB(255, 255, 255));
			break;
		}
		case str::hash("funinv"):
		{
			if (client->storage.m_FamilyID != 0 && client->storage.m_FamilyRank >= 1)
			{
				if (breaker.Params.length() >= 4 && breaker.Params.length() <= 25)
				{
					sql::ResultSet* res = client->serverInstance->SQLInstance()->get("SELECT DISTINCT * FROM users WHERE Username = ? AND Family = ? AND FamilyRank < ?",
						breaker.Params, client->storage.m_FamilyID, client->storage.m_FamilyRank);
					if (res != nullptr)
					{
						if (res->next())
						{
							std::list<ConnectedClient*> f_clients = client->serverInstance->FindUsersIf([&](ConnectedClient* f_client) {
								if (f_client->storage.m_LoggedIn == true && str::caseInSensStringCompare(f_client->storage.m_Name, breaker.Params)) return true;
								else return false;
							});
							if (f_clients.size() > 0)
							{
								ConnectedClient* f_client = f_clients.back();
								f_client->storage.m_FamilyID = 0;
								f_client->storage.m_FamilyRank = 0;
								f_client->SendClientMessage(str::format("{0} �������� ��� �� �����.", client->storage.m_Name), CHAT_RGB(255, 136, 77));
							}
							client->serverInstance->SQLInstance()->run("UPDATE users SET Family = 0, FamilyRank = 0 WHERE ID = ?", res->getInt("ID"));
							client->SendClientMessage(str::format("�� ��������� {0} �� �����.", res->getString("Username")), CHAT_RGB(255, 136, 77));
						}
						else client->SendClientMessage("������� �� ������ / �� � ����� / ���� ��� �� �����.", CHAT_RGB(255, 136, 77));
						delete res;
					}
					else client->SendClientMessage("��������� ������.", CHAT_RGB(255, 136, 77));
				}
				else client->SendClientMessage("���������: !funinv [���], ��� ������ �������� �� 4-25 ��������", CHAT_RGB(255, 255, 255));
			}
			else client->SendClientMessage("�� �� �������� � �����, ���� �� ���������/���.", CHAT_RGB(255, 255, 255));
			break;
		}
		case str::hash("fpromote"):
		{
			if (client->storage.m_FamilyID != 0 && client->storage.m_FamilyRank >= 2)
			{
				if (breaker.Params.length() >= 4 && breaker.Params.length() <= 25)
				{
					sql::ResultSet* res = client->serverInstance->SQLInstance()->get("SELECT DISTINCT * FROM users WHERE Username = ? AND Family = ? AND FamilyRank < ?",
						breaker.Params, client->storage.m_FamilyID, client->storage.m_FamilyRank);
					if (res != nullptr)
					{
						if (res->next())
						{
							int FamilySqlRank = res->getInt("FamilyRank");
							//
							std::list<ConnectedClient*> f_clients = client->serverInstance->FindUsersIf([&](ConnectedClient* f_client) {
								if (f_client->storage.m_LoggedIn == true && str::caseInSensStringCompare(f_client->storage.m_Name, breaker.Params)) return true;
								else return false;
							});
							if (f_clients.size() > 0)
							{
								ConnectedClient* f_client = f_clients.back();
								f_client->storage.m_FamilyRank = FamilySqlRank == 0 ? 1 : 0;
								f_client->SendClientMessage(str::format("{0} {1} ��� � �����.", client->storage.m_Name, FamilySqlRank == 0 ? "�������" : "�������"), FamilySqlRank == 0 ? CHAT_RGB(0, 204, 102) : CHAT_RGB(255, 136, 77));
							}
							client->serverInstance->SQLInstance()->run("UPDATE users SET FamilyRank = ? WHERE ID = ?", FamilySqlRank == 0 ? 1 : 0, res->getInt("ID"));
							client->SendClientMessage(str::format("�� {0} � ����� {1}", FamilySqlRank == 0 ? "�������" : "�������", res->getString("Username")), FamilySqlRank == 0 ? CHAT_RGB(0, 204, 102) : CHAT_RGB(255, 136, 77));
						}
						else client->SendClientMessage("������� �� ������ / �� � ����� / ���� ��� �� �����.", CHAT_RGB(255, 136, 77));
						delete res;
					}
					else client->SendClientMessage("��������� ������.", CHAT_RGB(255, 136, 77));
				}
				else client->SendClientMessage("���������: !fpromote [���], ��� ������ �������� �� 4-25 ��������", CHAT_RGB(255, 255, 255));
			}
			else client->SendClientMessage("�� �� �������� � �����, ���� �� ���������.", CHAT_RGB(255, 255, 255));
			break;
		}
		case str::hash("family"):
		{
			if (client->storage.m_FamilyID != 0)
			{
				std::list<FamilyStruct>::iterator FamIt = std::find_if(client->serverInstance->storage.families.begin(), client->serverInstance->storage.families.end(), [&](FamilyStruct& famTemp) {
					if (famTemp.id == client->storage.m_FamilyID) return true;
					else return false;
				});
				client->ShowClientDialog(4, DialogStyles::DIALOG_STYLE_LIST, str::format("{FFFFFF}���� ����� {{0}}[ {1} ]", str::NumToHex(FamIt->color), FamIt->name),
					"{FFFFFF}1. �������� �������� �����\n2. �������� ���� ����\n3. ������ �����\n4. {FF8C00}�������� �����",
					"�������", "�������");
			}
			else client->ShowClientDialog(8, DialogStyles::DIALOG_STYLE_INPUT, "{FFCD00}�������� �����",
					"{FFFFFF}�� ������ ������� ���� �����\n\n������� �������� �������� �����\n�������� ������ ���� �� 5 �� 30 ��������\n", "�������", "������");
			break;
		}
		default:
		{
			client->SendClientMessage("������ ������� �� ����������.", CHAT_RGB(255, 255, 255));
			return;
		}
	}
}
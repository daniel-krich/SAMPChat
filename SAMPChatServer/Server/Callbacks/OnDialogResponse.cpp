#include "../../main.h"

void OnDialogResponse(ConnectedClient* client, uint16_t dialogId, uint16_t itemId, uint8_t buttonId, std::string response)
{
	if (client->storage.t_CurrentDialogId != dialogId)
	{
		if(client->storage.m_LoggedIn == false) PrintGUI(str::format("{0} possibly sending fake dialogs", client->GetIP()));
		else PrintGUI(str::format("{0} ({1}) possibly sending fake dialogs", client->storage.m_Name, client->GetIP()));
		client->Close();
		return;
	}
	client->storage.t_CurrentDialogId = 0;
	//
	switch (dialogId)
	{
		case 1:
		{
			if (buttonId == DialogResponseButton::DialogAccept)
			{
				if (response.length() <= 25 && response.length() >= 4)
				{
					sql::ResultSet* res = client->serverInstance->SQLInstance()->get("SELECT DISTINCT * FROM users WHERE Username = ?", response);
					if (res != nullptr)
					{
						if (res->next())
						{
							client->storage.t_TempName = response;
							client->ShowClientDialog(3, DialogStyles::DIALOG_STYLE_PASSWORD, "{FFCC00}Authorization",
								str::format("{FFFFFF}Welcome to SAMPChat!\n\n{FFFFFF}Current nickname: {FFCC00}{0}\n{FFFFFF}Enter your account password:\n", client->storage.t_TempName),
								"Login", "Cancel");
						}
						else
						{
							client->storage.t_TempName = response;
							client->ShowClientDialog(2, DialogStyles::DIALOG_STYLE_INPUT, "{0066CC}Registration",
								str::format("{FFFFFF}Welcome to SAMPChat!\n\n{FFFFFF}Current nickname: {FFCC00}{0}\n{FFFFFF}Enter any password you want to use:\n", client->storage.t_TempName),
								"Next", "Cancel");
						}
						delete res;
					}
				}
				else
				{
					client->ShowClientDialog(1, DialogStyles::DIALOG_STYLE_INPUT, "{FFCC00}Authorization",
						str::format("{FFFFFF}Welcome to SAMPChat!\n\n{FFFFFF}IP address: {FFCC00}{0}\n{FFFFFF}Enter your account name:\n{ff884d}* 4-25 characters", client->GetIP()),
						"Next", "Cancel");
				}
			}
			break;
		}
		case 2:
		{
			if (buttonId == DialogResponseButton::DialogAccept)
			{
				if (response.length() <= 25 && response.length() >= 6)
				{
					if (client->serverInstance->SQLInstance()->run("INSERT INTO users(Username, Pswd, Family, FamilyRank) values(?, ?, 0, 0)", client->storage.t_TempName, response))
					{
						client->ShowClientDialog(3, DialogStyles::DIALOG_STYLE_PASSWORD, "{FFCC00}Authorization",
							str::format("{FFFFFF}Welcome to SAMPChat!\n\n{FFFFFF}Current nickname: {FFCC00}{0}\n{FFFFFF}Enter your account password:\n", client->storage.t_TempName),
							"Login", "Cancel");
					}
					else client->SendClientMessage("An error occurred while registering, please try again.", CHAT_RGB(255, 51, 51));
				}
				else client->ShowClientDialog(2, DialogStyles::DIALOG_STYLE_INPUT, "{0066CC}Registration",
						str::format("{FFFFFF}Welcome to SAMPChat!\n\n{FFFFFF}Current nickname: {FFCC00}{0}\n{FFFFFF}Enter any password you will use:\n{ff884d}* 5-25 characters\n", client->storage.t_TempName),
						"Next", "Cancel");
			}
			break;
		}
		case 3:
		{
			if(buttonId == DialogResponseButton::DialogAccept)
			{
				sql::ResultSet* res = client->serverInstance->SQLInstance()->get("SELECT DISTINCT * FROM users WHERE Username = ? AND Pswd = ?", client->storage.t_TempName, response);
				if (res != nullptr)
				{
					if (res->next())
					{

						if (client->serverInstance->FindUsersIf([&](ConnectedClient* f_client) {
							if (f_client->storage.m_LoggedIn == true && str::caseInSensStringCompare(f_client->storage.m_Name, client->storage.t_TempName)) return true;
							else return false;
						}).size() == 0)
						{
							client->storage.m_LoggedIn = true;
							client->storage.m_ID = res->getInt("ID");
							client->storage.m_Name = res->getString("Username");
							client->storage.m_FamilyID = res->getInt("Family");
							client->storage.m_FamilyRank = res->getInt("FamilyRank");
							client->storage.t_TempName.clear();
							//
							client->SendClientMessage(str::format("Greetings, {FFFFFF}{0} (Account ID {1}).", client->storage.m_Name, client->storage.m_ID), CHAT_RGB(0, 204, 102));
						}
						else client->SendClientMessage("A user with this name is already connected to the server.", CHAT_RGB(255, 153, 102));
					}
					else client->ShowClientDialog(3, DialogStyles::DIALOG_STYLE_PASSWORD, "{FFCC00}Authorization",
							str::format("{FFFFFF}Welcome to SAMPChat!\n\n{FFFFFF}Current nickname: {FFCC00}{0}\n{FFFFFF}Enter your account password:\n{ff884d}* Incorrect password\n", client->storage.t_TempName),
							"Login", "Cancel");
					delete res;
				}
			}
			break;
		}
		case 4:
		{
			if (buttonId == DialogResponseButton::DialogAccept)
			{
				if (client->storage.m_FamilyID == 0) break;
				//
				switch (itemId)
				{
				case 0: //Change fam name
				{
					if (client->storage.m_FamilyRank >= 2)
					{
						client->ShowClientDialog(5, DialogStyles::DIALOG_STYLE_INPUT, "{FFCD00}Rename family",
							"{FFFFFF}You can rename your family\n\nEnter a new family name\nThe name must be between 5 and 30 characters\n",
							"Edit", "Cancel");
					}
					else client->SendClientMessage("Only available to family creators.", CHAT_RGB(255, 255, 255));
					break;
				}
				case 1: //Change chat color
				{
					if (client->storage.m_FamilyRank >= 1)
					{
						client->ShowClientDialog(6, DialogStyles::DIALOG_STYLE_INPUT, "{FFCD00}Change chat color",
							"{FFFFFF}You can set any color you like.\n\nColor must be entered in HEX format 6 characters, for example {FFCC00}\"FFCC00\"\n{FFFFFF}The color code can be found at {00cc66}ColorPicker {FFFFFF} on the web.\n",
							"Edit", "Cancel");
					}
					else client->SendClientMessage("Available only to the creators of families and their deputies.", CHAT_RGB(255, 255, 255));
					break;
				}
				case 2: //Online fam members
				{
					int memberCount = 0;
					std::list<FamilyStruct>::iterator famIt = std::find_if(client->serverInstance->storage.families.begin(), client->serverInstance->storage.families.end(), [&](FamilyStruct& t_Fam) {
						if (t_Fam.id == client->storage.m_FamilyID) return true;
						else return false;
					});
					for (ConnectedClient* f_client : client->serverInstance->ConnectedUsers())
					{
						if (f_client->storage.m_FamilyID == client->storage.m_FamilyID)
						{
							client->SendClientMessage(str::format("{0}. {FFFFFF}{1}", ++memberCount, f_client->storage.m_Name), famIt->color);
						}
					}
					client->SendClientMessage(str::format("Online members - {FFFFFF}{0}", memberCount), famIt->color);
					break;
				}
				case 3: //Leave fam
				{
					client->ShowClientDialog(7, DialogStyles::DIALOG_STYLE_MSGBOX, "{FF6200}Confirm action", "\n{FFFFFF}You are about to leave your current family\nIf you wish to continue, press {FF6200}\"Next\".\n\n{CECECE}Note: If you are the creator of the family, your family will deleted.",
						"Next", "Cancel");
					break;
				}
				default: break;
				}
			}
			break;
		}
		case 5:
		{
			if (buttonId == DialogResponseButton::DialogAccept)
			{
				if (client->storage.m_FamilyRank >= 2 && client->storage.m_FamilyID != 0)
				{
					if (response.length() >= 5 && response.length() <= 30)
					{
						
						std::list<FamilyStruct>::iterator famIt = std::find_if(client->serverInstance->storage.families.begin(), client->serverInstance->storage.families.end(), [&](FamilyStruct& t_Fam){
							if (str::caseInSensStringCompare(t_Fam.name, response)) return true;
							else return false;
						});
						if (famIt == client->serverInstance->storage.families.end())
						{
							client->serverInstance->SQLInstance()->run("UPDATE families SET Name = ? WHERE ID = ?", response, client->storage.m_FamilyID);
							for (FamilyStruct& elem : client->serverInstance->storage.families)
							{
								if (elem.id == client->storage.m_FamilyID)
								{
									elem.name = response;
									break;
								}
							}
							client->SendClientMessage(str::format("The family name has been successfully changed to {00b359}{0}", response), CHAT_RGB(255, 255, 255));
						}
						else client->SendClientMessage("This family is already registered on the server.", CHAT_RGB(255, 255, 255));
					}
					else client->SendClientMessage("Family name can be 5-30 characters long.", CHAT_RGB(255, 255, 255));
				}
			}
			break;
		}
		case 6:
		{
			if (buttonId == DialogResponseButton::DialogAccept)
			{
				if (client->storage.m_FamilyRank >= 1 && client->storage.m_FamilyID != 0)
				{
					if (response.length() == 6)
					{
						std::list<FamilyStruct>::iterator famIt = std::find_if(client->serverInstance->storage.families.begin(), client->serverInstance->storage.families.end(), [&](FamilyStruct& f_Fam) {
							if (f_Fam.id == client->storage.m_FamilyID) return true;
							else return false;
						});
						famIt->color = str::HexToNum(response);
						client->serverInstance->SQLInstance()->run("UPDATE families SET Color = ? WHERE ID = ?", str::HexToNum(response), client->storage.m_FamilyID);
						client->SendClientMessage("You have successfully changed the family chat color.", str::HexToNum(response));
					}
					else client->SendClientMessage("Only HEX format is supported, 6 characters.", CHAT_RGB(255, 255, 255));
				}
			}
			break;
		}
		case 7:
		{
			if (buttonId == DialogResponseButton::DialogAccept)
			{
				if (client->storage.m_FamilyID != 0)
				{
					if (client->storage.m_FamilyRank >= 2)
					{
						int tempFamilyID = client->storage.m_FamilyID;
						for (ConnectedClient* f_client : client->serverInstance->ConnectedUsers())
						{
							if (f_client->storage.m_FamilyID == tempFamilyID)
							{
								f_client->storage.m_FamilyID = 0;
								f_client->storage.m_FamilyRank = 0;
								f_client->SendClientMessage("Your family has been deleted by the creator, all its members have been removed.", CHAT_RGB(255, 136, 77));
							}
						}
						client->serverInstance->storage.families.erase(std::remove_if(client->serverInstance->storage.families.begin(), client->serverInstance->storage.families.end(), [&](FamilyStruct& f_Fam) {
							if (f_Fam.id == tempFamilyID) return true;
							else return false;
						}), client->serverInstance->storage.families.end());
						client->serverInstance->SQLInstance()->run("UPDATE users SET Family = 0, FamilyRank = 0 WHERE Family = ?", tempFamilyID);
						client->serverInstance->SQLInstance()->run("DELETE from families WHERE ID = ?", tempFamilyID);
					}
					else
					{
						client->storage.m_FamilyID = 0;
						client->storage.m_FamilyRank = 0;
						client->serverInstance->SQLInstance()->run("UPDATE users SET Family = 0, FamilyRank = 0 WHERE Username = ?", client->storage.m_Name);
						client->SendClientMessage("You have successfully left your family.", CHAT_RGB(255, 255, 255));
					}
				}
			}
			break;
		}
		case 8:
		{
			if (buttonId == DialogResponseButton::DialogAccept)
			{
				if (client->storage.m_FamilyID == 0)
				{
					if (response.length() >= 5 && response.length() <= 30)
					{
						std::list<FamilyStruct>::iterator famIt = std::find_if(client->serverInstance->storage.families.begin(), client->serverInstance->storage.families.end(), [&](FamilyStruct& t_Fam) {
							if (str::caseInSensStringCompare(t_Fam.name, response)) return true;
							else return false;
						});
						if (famIt == client->serverInstance->storage.families.end())
						{
							if (client->serverInstance->SQLInstance()->run("INSERT INTO families(Name, Color) VALUES(?, ?)", response, str::HexToNum("FFFFFF")))
							{
								sql::ResultSet* res = client->serverInstance->SQLInstance()->get("SELECT DISTINCT * FROM families WHERE Name = ?", response);
								if (res != nullptr)
								{
									if (res->next())
									{
										//Update account
										client->serverInstance->SQLInstance()->run("UPDATE users SET Family = ?, FamilyRank = 2 WHERE Username = ?", res->getInt("ID"), client->storage.m_Name);
										client->storage.m_FamilyID = res->getInt("ID");
										client->storage.m_FamilyRank = 2;
										//Update server storage
										FamilyStruct t_Fam = { res->getInt("ID"), res->getString("Name"), (DWORD)res->getInt64("Color") };
										client->serverInstance->storage.families.push_back(t_Fam);
										client->SendClientMessage(str::format("Created a new family {00b359}{0}", response), CHAT_RGB(255, 255, 255));
									}
									else client->SendClientMessage("An error has occurred #3", CHAT_RGB(255, 255, 255));
									delete res;
								}
								else client->SendClientMessage("An error has occurred #2", CHAT_RGB(255, 255, 255));
							}
							else client->SendClientMessage("An error has occurred #1", CHAT_RGB(255, 255, 255));
						}
						else client->SendClientMessage("This family is already registered on the server.", CHAT_RGB(255, 255, 255));
					}
					else client->SendClientMessage("Family name can be 5-30 characters long.", CHAT_RGB(255, 255, 255));
				}
			}
			break;
		}
		case 9:
		{
			if (buttonId == DialogResponseButton::DialogAccept)
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
							client->storage.m_FamilyID = f_client->storage.m_FamilyID;
							client->storage.m_FamilyRank = 0;
							client->serverInstance->SQLInstance()->run("UPDATE users SET Family = ?, FamilyRank = ? WHERE ID = ?",
								client->storage.m_FamilyID, client->storage.m_FamilyRank, client->storage.m_ID);
							f_client->SendClientMessage(str::format("{0} accepted the invitation.", client->storage.m_Name), CHAT_RGB(0, 204, 102));
							client->SendClientMessage("You accepted the invitation.", CHAT_RGB(0, 204, 102));
						}
						else client->SendClientMessage("An error has occurred, please try again.", CHAT_RGB(255, 136, 77));
					}
					else client->SendClientMessage("An error has occurred, please try again.", CHAT_RGB(255, 136, 77));
					client->storage.t_TempUserId_Invite = 0;
				}
				else client->SendClientMessage("Offer no longer valid.", CHAT_RGB(255, 136, 77));
			}
			else
			{
				if (client->storage.t_TempUserId_Invite != 0)
				{
					std::list<ConnectedClient*> f_clients = client->serverInstance->FindUsersIf([&](ConnectedClient* f_client) {
						if (f_client->storage.m_ID == client->storage.t_TempUserId_Invite) return true;
						else return false;
					});
					if (f_clients.size() > 0)
					{
						ConnectedClient* f_client = f_clients.back();
						f_client->SendClientMessage(str::format("{0} refused the invitation", client->storage.m_Name), CHAT_RGB(255, 136, 77));
					}
				}
				client->storage.t_TempUserId_Invite = 0;
				client->SendClientMessage("You refused the invitation.", CHAT_RGB(255, 136, 77));
			}
			break;
		}
		default: return;
	}
}
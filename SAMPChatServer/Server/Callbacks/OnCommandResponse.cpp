#include "../../main.h"

void OnCommandResponse(ConnectedClient* client, std::string data)
{
	str::Breaker breaker = str::breakOnSpace(data);
	if (client->storage.m_LoggedIn == false && breaker.Cmd != "auth")
	{
		client->SendClientMessage("You are not authorized, !auth.", CHAT_RGB(255, 136, 77));
		return;
	}
	switch (str::hash(breaker.Cmd.c_str()))
	{
		case str::hash("auth"):
		{
			if (client->storage.m_LoggedIn == false)
			{
				client->ShowClientDialog(1, DialogStyles::DIALOG_STYLE_INPUT, "{FFCC00}Authorization",
					str::format("{FFFFFF}Welcome to SAMPChat!\n\n{FFFFFF}IP address: {FFCC00}{0}\n{FFFFFF}Enter your account name:\n", client->GetIP()),
					"Next", "Cancel");
			}
			else client->SendClientMessage(str::format("You are already logged in, {FFFFFF}{0}.", client->storage.m_Name), CHAT_RGB(0, 204, 102));
			break;
		}
		case str::hash("help"):
		{
			client->ShowClientDialog(0, DialogStyles::DIALOG_STYLE_MSGBOX, "{00cc70}Commands",
				"\n{FFFFFF}!auth\t\t{00cc70}Login/Register\n{FFFFFF}!g\t\t{00cc70}Global chat\n{FFFFFF}!f\t\t{00cc70}Family chat\n{FFFFFF}!finvdetails\t{00cc70}View offer details\n{FFFFFF}!finv\t\t{00cc70}Invite to family\n{FFFFFF}!funinv\t\t{00cc70}Exclude from family\n{FFFFFF}!fpromote\t{00cc70}Promote/Lower Family Rank (Creators only)\n{FFFFFF}!family\t\t{00cc70}Create a Family/Family Menu",
				"Close");
			break;
		}
		case str::hash("g"):
		{
			if (breaker.Params.length() > 0)
			{
				for (ConnectedClient* f_client : client->serverInstance->ConnectedUsers())
				{
					if (f_client->storage.m_LoggedIn == true) f_client->SendClientMessage(str::format("[*] {0}: {FFFFFF}{1}", client->storage.m_Name, breaker.Params), CHAT_RGB(255, 136, 77));
					else f_client->SendClientMessage("[*] A message was sent by someone, log in to view.", CHAT_RGB(255, 136, 77));
				}
			}
			else client->SendClientMessage("Use: !g [text]", CHAT_RGB(255, 255, 255));
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
				else client->SendClientMessage("Use: !f [text]", CHAT_RGB(255, 255, 255));
			}
			else client->SendClientMessage("You are not part of a clan.", CHAT_RGB(255, 255, 255));
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
						client->ShowClientDialog(9, DialogStyles::DIALOG_STYLE_MSGBOX, str::format("{{0}}Offer", str::NumToHex(famIt->color)),
							str::format("{FFFFFF}Inviter:\t\t{{0}}{1}\n{FFFFFF}Family:\t\t\t{{2}}{3}\n", str::NumToHex(famIt->color), f_client->storage.m_Name, str::NumToHex(famIt->color), famIt->name), "Accept", "Reject");
					}
					else
					{
						client->SendClientMessage("The offer is no longer valid.", CHAT_RGB(255, 136, 77));
						client->storage.t_TempUserId_Invite = 0;
					}
				}
				else
				{
					client->SendClientMessage("The offer is no longer valid.", CHAT_RGB(255, 136, 77));
					client->storage.t_TempUserId_Invite = 0;
				}
			}
			else client->SendClientMessage("No new offers.", CHAT_RGB(255, 136, 77));
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
									client->SendClientMessage(str::format("You have invited {00CC70}{0} {FFFFFF} to join your family.", f_client->storage.m_Name), CHAT_RGB(255, 255, 255));
									//
									f_client->storage.t_TempUserId_Invite = client->storage.m_ID;
									f_client->SendClientMessage(str::format("Received a new invitation to join a clan from {00CC70}{0}.", client->storage.m_Name), CHAT_RGB(255, 255, 255));
									f_client->SendClientMessage("To view the invitation, type {00CC70}!finvdetails", CHAT_RGB(255, 255, 255));
								}
								else if (f_client->storage.t_TempUserId_Invite == client->storage.m_ID)
								{
									client->SendClientMessage(str::format("You have withdrawn the offer from {0}.", f_client->storage.m_Name), CHAT_RGB(255, 136, 77));
									//
									f_client->storage.t_TempUserId_Invite = 0;
									f_client->SendClientMessage(str::format("{0} withdrew the offer.", client->storage.m_Name), CHAT_RGB(255, 136, 77));
								}
								else client->SendClientMessage("Someone has already offered this user to join a clan, wait a couple of seconds.", CHAT_RGB(255, 255, 255));
							}
							else client->SendClientMessage("This user is already in a family.", CHAT_RGB(255, 255, 255));
						}
						else client->SendClientMessage("This user is not authorized.", CHAT_RGB(255, 255, 255));
					}
					else client->SendClientMessage("This user is not online.", CHAT_RGB(255, 255, 255));
				}
				else client->SendClientMessage("Use: !finv [name], name must be 4-25 characters long", CHAT_RGB(255, 255, 255));
			}
			else client->SendClientMessage("You are not a member of a family, or not a creator/deputy.", CHAT_RGB(255, 255, 255));
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
								f_client->SendClientMessage(str::format("{0} excluded you from the family.", client->storage.m_Name), CHAT_RGB(255, 136, 77));
							}
							client->serverInstance->SQLInstance()->run("UPDATE users SET Family = 0, FamilyRank = 0 WHERE ID = ?", res->getInt("ID"));
							client->SendClientMessage(str::format("You have removed {0} from your family.", res->getString("Username")), CHAT_RGB(255, 136, 77));
						}
						else client->SendClientMessage("Account not found / not in the family / higher than your rank.", CHAT_RGB(255, 136, 77));
						delete res;
					}
					else client->SendClientMessage("Error occured.", CHAT_RGB(255, 136, 77));
				}
				else client->SendClientMessage("Use: !funinv [name], name must be 4-25 characters long", CHAT_RGB(255, 255, 255));
			}
			else client->SendClientMessage("You are not a member of a family, or not a creator/deputy.", CHAT_RGB(255, 255, 255));
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
								f_client->SendClientMessage(str::format("{0} {1} your rank.", client->storage.m_Name, FamilySqlRank == 0 ? "raised" : "lowered"), FamilySqlRank == 0 ? CHAT_RGB(0, 204, 102) : CHAT_RGB(255, 136, 77));
							}
							client->serverInstance->SQLInstance()->run("UPDATE users SET FamilyRank = ? WHERE ID = ?", FamilySqlRank == 0 ? 1 : 0, res->getInt("ID"));
							client->SendClientMessage(str::format("You {0} in rank {1}", FamilySqlRank == 0 ? "raised" : "lowered", res->getString("Username")), FamilySqlRank == 0 ? CHAT_RGB(0, 204, 102) : CHAT_RGB(255, 136, 77));
						}
						else client->SendClientMessage("Account not found / not in the family / higher than your rank.", CHAT_RGB(255, 136, 77));
						delete res;
					}
					else client->SendClientMessage("Error occured.", CHAT_RGB(255, 136, 77));
				}
				else client->SendClientMessage("Use: !fpromote [name], name must be 4-25 characters long", CHAT_RGB(255, 255, 255));
			}
			else client->SendClientMessage("You are not a member of a family, or not a creator.", CHAT_RGB(255, 255, 255));
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
				client->ShowClientDialog(4, DialogStyles::DIALOG_STYLE_LIST, str::format("{FFFFFF}Your family {{0}}[ {1} ]", str::NumToHex(FamIt->color), FamIt->name),
					"{FFFFFF}1. Change family name\n2. Change chat color\n3. Online members\n4. {FF8C00}Leave family",
					"Select", "Close");
			}
			else client->ShowClientDialog(8, DialogStyles::DIALOG_STYLE_INPUT, "{FFCD00}Create family",
					"{FFFFFF}You can create your own family\n\nEnter the desired family name\nThe name must be between 5 and 30 characters\n", "Create", "Cancel");
			break;
		}
		default:
		{
			client->SendClientMessage("Unknown command.", CHAT_RGB(255, 255, 255));
			return;
		}
	}
}
#include "../../main.h"

void OnDialogResponse(ConnectedClient* client, uint16_t dialogId, uint16_t itemId, uint8_t buttonId, std::string response)
{
	if (client->storage.t_CurrentDialogId != dialogId)
	{
		if(client->storage.m_LoggedIn == false) PrintGUI(str::format("{0} возможно отправляет фейк диалоги", client->GetIP()));
		else PrintGUI(str::format("{0} ({1}) возможно отправляет фейк диалоги", client->storage.m_Name, client->GetIP()));
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
							client->ShowClientDialog(3, DialogStyles::DIALOG_STYLE_PASSWORD, "{FFCC00}Авторизация",
								str::format("{FFFFFF}Добро пожаловать на SAMPChat!\n\n{FFFFFF}Текущий ник: {FFCC00}{0}\n{FFFFFF}Введите свой пароль от аккаунта:\n", client->storage.t_TempName),
								"Войти", "Отмена");
						}
						else
						{
							client->storage.t_TempName = response;
							client->ShowClientDialog(2, DialogStyles::DIALOG_STYLE_INPUT, "{0066CC}Регистрация",
								str::format("{FFFFFF}Добро пожаловать на SAMPChat!\n\n{FFFFFF}Текущий ник: {FFCC00}{0}\n{FFFFFF}Введите любой пароль которым вы будете пользоваться:\n", client->storage.t_TempName),
								"Далее", "Отмена");
						}
						delete res;
					}
				}
				else
				{
					client->ShowClientDialog(1, DialogStyles::DIALOG_STYLE_INPUT, "{FFCC00}Авторизация",
						str::format("{FFFFFF}Добро пожаловать на SAMPChat!\n\n{FFFFFF}IP адрес: {FFCC00}{0}\n{FFFFFF}Введите имя вашего аккаунта:\n{ff884d}* 4-25 символов", client->GetIP()),
						"Далее", "Отмена");
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
						client->ShowClientDialog(3, DialogStyles::DIALOG_STYLE_PASSWORD, "{FFCC00}Авторизация",
							str::format("{FFFFFF}Добро пожаловать на SAMPChat!\n\n{FFFFFF}Текущий ник: {FFCC00}{0}\n{FFFFFF}Введите свой пароль от аккаунта:\n", client->storage.t_TempName),
							"Войти", "Отмена");
					}
					else client->SendClientMessage("Произошла ошибка при регистрации, попробуйте заново.", CHAT_RGB(255, 51, 51));
				}
				else client->ShowClientDialog(2, DialogStyles::DIALOG_STYLE_INPUT, "{0066CC}Регистрация",
						str::format("{FFFFFF}Добро пожаловать на SAMPChat!\n\n{FFFFFF}Текущий ник: {FFCC00}{0}\n{FFFFFF}Введите любой пароль которым вы будете пользоваться:\n{ff884d}* 5-25 символов\n", client->storage.t_TempName),
						"Далее", "Отмена");
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
							client->SendClientMessage(str::format("Добро пожаловать, {FFFFFF}{0} (Аккаунт №{1}).", client->storage.m_Name, client->storage.m_ID), CHAT_RGB(0, 204, 102));
						}
						else client->SendClientMessage("Пользователь с таким именем уже подключен к серверу.", CHAT_RGB(255, 153, 102));
					}
					else client->ShowClientDialog(3, DialogStyles::DIALOG_STYLE_PASSWORD, "{FFCC00}Авторизация",
							str::format("{FFFFFF}Добро пожаловать на SAMPChat!\n\n{FFFFFF}Текущий ник: {FFCC00}{0}\n{FFFFFF}Введите свой пароль от аккаунта:\n{ff884d}* Пароль неверный\n", client->storage.t_TempName),
							"Войти", "Отмена");
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
				case 0: //Изменить название
				{
					if (client->storage.m_FamilyRank >= 2)
					{
						client->ShowClientDialog(5, DialogStyles::DIALOG_STYLE_INPUT, "{FFCD00}Переименование семьи",
							"{FFFFFF}Вы можете переименовать свою семью\n\nВведите новое название семьи\nНазвание должно быть от 5 до 30 символов\n",
							"Изменить", "Отмена");
					}
					else client->SendClientMessage("Доступно только создателям семей.", CHAT_RGB(255, 255, 255));
					break;
				}
				case 1: //Поменять цвет фамы
				{
					if (client->storage.m_FamilyRank >= 1)
					{
						client->ShowClientDialog(6, DialogStyles::DIALOG_STYLE_INPUT, "{FFCD00}Изменение цвета чата",
							"{FFFFFF}Вы можете установить любой цвет на ваш вкус.\n\nЦвет нужно ввести в формате HEX 6 символов, например {FFCC00}\"FFCC00\"\n{FFFFFF}Код цвета можно найти на {00cc66}ColorPicker {FFFFFF}в интернете.\n",
							"Изменить", "Отмена");
					}
					else client->SendClientMessage("Доступно только создателям семей и их заместителям.", CHAT_RGB(255, 255, 255));
					break;
				}
				case 2: //Челы онлайн
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
					client->SendClientMessage(str::format("Всего онлайн - {FFFFFF}{0}", memberCount), famIt->color);
					break;
				}
				case 3: //Покинуть фаму
				{
					client->ShowClientDialog(7, DialogStyles::DIALOG_STYLE_MSGBOX, "{FF6200}Подтверждение действие", "\n{FFFFFF}Вы собираетесь уйти из вашей текущей семьи в которой вы состоите\nЕсли вы желаете продолжить нажмите {FF6200}\"Далее\".\n\n{CECECE}Примечание: Если вы руководитель семьи, то ваша семья будет удалена при покидании.",
						"Далее", "Отмена");
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
							client->SendClientMessage(str::format("Имя семьи было успешно изменено на {00b359}{0}", response), CHAT_RGB(255, 255, 255));
						}
						else client->SendClientMessage("Такая семья уже зарегистрирована на сервере.", CHAT_RGB(255, 255, 255));
					}
					else client->SendClientMessage("Имя семьи может состоять из 5-30 символов.", CHAT_RGB(255, 255, 255));
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
						client->SendClientMessage("Вы успешно изменили цвет чата семьи.", str::HexToNum(response));
					}
					else client->SendClientMessage("Поддерживается только HEX-формат, 6 символов.", CHAT_RGB(255, 255, 255));
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
								f_client->SendClientMessage("Ваша семья была удалена создателём, все её участники были аннулированы.", CHAT_RGB(255, 136, 77));
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
						client->SendClientMessage("Вы успешно покинули семью.", CHAT_RGB(255, 255, 255));
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
										client->SendClientMessage(str::format("Создана новая семья {00b359}{0}", response), CHAT_RGB(255, 255, 255));
									}
									else client->SendClientMessage("Произошла ошибка #3", CHAT_RGB(255, 255, 255));
									delete res;
								}
								else client->SendClientMessage("Произошла ошибка #2", CHAT_RGB(255, 255, 255));
							}
							else client->SendClientMessage("Произошла ошибка #1", CHAT_RGB(255, 255, 255));
						}
						else client->SendClientMessage("Такая семья уже зарегистрирована на сервере.", CHAT_RGB(255, 255, 255));
					}
					else client->SendClientMessage("Имя семьи может состоять из 5-30 символов.", CHAT_RGB(255, 255, 255));
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
							f_client->SendClientMessage(str::format("{0} принял приглашение.", client->storage.m_Name), CHAT_RGB(0, 204, 102));
							client->SendClientMessage("Вы приняли приглашение.", CHAT_RGB(0, 204, 102));
						}
						else client->SendClientMessage("Произошла ошибка, попробуйте заного.", CHAT_RGB(255, 136, 77));
					}
					else client->SendClientMessage("Произошла ошибка, попробуйте заного.", CHAT_RGB(255, 136, 77));
					client->storage.t_TempUserId_Invite = 0;
				}
				else client->SendClientMessage("Предложение уже не действует.", CHAT_RGB(255, 136, 77));
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
						f_client->SendClientMessage(str::format("{0} отказался от приглашение", client->storage.m_Name), CHAT_RGB(255, 136, 77));
					}
				}
				client->storage.t_TempUserId_Invite = 0;
				client->SendClientMessage("Вы отказались от приглашение.", CHAT_RGB(255, 136, 77));
			}
			break;
		}
		default: return;
	}
}
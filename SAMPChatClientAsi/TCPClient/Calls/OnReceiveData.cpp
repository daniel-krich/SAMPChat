#include "../main.h"

void TCP::OnReceiveDataCall(const rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator>& data)
{
	try
	{
		if (!data.HasMember("type") || !data["type"].IsInt()) return;
		switch (data["type"].GetInt())
		{
			case SendMessageType::SendClientMessage:
			{
				if ((!data.HasMember("message") || !data["message"].IsString()) ||
					(!data.HasMember("color")	|| !data["color"].IsUint64())) return;
				addToChatWindow(data["message"].GetString(), data["color"].GetUint64());
				break;
			}
			case SendMessageType::ShowClientDialog:
			{
				if ((!data.HasMember("dialogId")		|| !data["dialogId"].IsInt())			||
					(!data.HasMember("dialogType")		|| !data["dialogType"].IsInt())			||
					(!data.HasMember("dialogHeader")	|| !data["dialogHeader"].IsString())	||
					(!data.HasMember("dialogText")		|| !data["dialogText"].IsString())		||
					(!data.HasMember("dialogButton1")	|| !data["dialogButton1"].IsString())	||
					(!data.HasMember("dialogButton2")	|| !data["dialogButton2"].IsString())) return;
				client->CurrentDialogID = data["dialogId"].GetInt();
				showSampDialog(0, data["dialogId"].GetInt(), data["dialogType"].GetInt(),
					data["dialogHeader"].GetString(), data["dialogText"].GetString(),
					data["dialogButton1"].GetString(), data["dialogButton2"].GetString());
				break;
			}
			default: return;
		}
	}
	catch (std::exception&)
	{
		//...
	}
}
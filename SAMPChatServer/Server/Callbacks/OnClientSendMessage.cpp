#include "../../main.h"

void OnClientSendMessage(ConnectedClient* client, const rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator>& data)
{
	if (!data.HasMember("type") || !data["type"].IsInt()) return;
	//
	switch (data["type"].GetInt())
	{
		case ResponseMessageType::CommandResponse:
		{
			if (!data.HasMember("message") || !data["message"].IsString()) return;
			OnCommandResponse(client, data["message"].GetString());
			break;
		}
		case ResponseMessageType::DialogResponse:
		{
			if ((!data.HasMember("dialogId") || !data["dialogId"].IsUint()) ||
				(!data.HasMember("itemId")   || !data["itemId"].IsUint())   ||
				(!data.HasMember("buttonId") || !data["buttonId"].IsUint()) ||
				(!data.HasMember("response") || !data["response"].IsString())) return;
			OnDialogResponse(client, data["dialogId"].GetUint(), data["itemId"].GetUint(), data["buttonId"].GetUint(),
				data["response"].GetString());
			break;
		}
		default: return;
	}
}

void OnClientSendMessageHandle(ConnectedClient* client, const rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator>& data)
{
	try
	{
		OnClientSendMessage(client, data);
	}
	catch (const std::exception& ex)
	{
		PrintGUI(str::format("OnClientSendMessage crash ({0}):", client->GetIP()));
		PrintGUI(ex.what());
		client->Close();
	}
}
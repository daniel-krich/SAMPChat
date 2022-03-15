#include "../main.h"

void HandleDialogResponse(WORD wDialogID, BYTE byteButtonID, WORD wItemID, const char* szResponseText)
{
	if (wDialogID == client->CurrentDialogID && wDialogID != 0)
	{
		client->CurrentDialogID = 0;
		using JsonDocument = rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator>;
		JsonDocument Empty;
		JsonDocument DR;
		DR.SetObject();
		DR.AddMember("type", TCP::ResponseMessageType::DialogResponse, DR.GetAllocator());
		DR.AddMember("dialogId", wDialogID, DR.GetAllocator());
		DR.AddMember("itemId", wItemID, DR.GetAllocator());
		DR.AddMember("buttonId", byteButtonID, DR.GetAllocator());
		DR.AddMember("response", std::string(szResponseText), DR.GetAllocator());
		rapidjson::StringBuffer bufferDR;
		rapidjson::Writer<rapidjson::StringBuffer> writer(bufferDR);
		DR.Accept(writer);
		client->Send(bufferDR.GetString());
		DR.Swap(Empty);
	}
}
#include "../main.h"

void HookRPCIN(unsigned char byteRPCID, RPCParameters* rpcParams, void(*functionPointer) (RPCParameters*))
{
	functionPointer(rpcParams);
}

bool HookRPCOut(int* uniqueID, BitStream* parameters, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp)
{
	if (*uniqueID == RPCEnumeration::RPC_Chat)
	{
		BYTE* messageLength = new BYTE;
		parameters->Read((char*)messageLength, 1);
		char* Message = new char[*messageLength+1];
		Message[*messageLength] = '\0';
		parameters->Read(Message, *messageLength);
		//
		if (Message[0] == '!' && strlen(Message) > 1)// && client->Connection == TCP::ConnectionStatus::Connected)
		{
			using JsonDocument = rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator>;
			JsonDocument Empty;
			JsonDocument CR;
			CR.SetObject();
			CR.AddMember("type", TCP::ResponseMessageType::CommandResponse, CR.GetAllocator());
			CR.AddMember("message", std::string((char*)&Message[1]), CR.GetAllocator());
			rapidjson::StringBuffer bufferCR;
			rapidjson::Writer<rapidjson::StringBuffer> writer(bufferCR);
			CR.Accept(writer);
			client->Send(bufferCR.GetString());
			CR.Swap(Empty);
			//
			delete[] Message;
			delete messageLength;
			return false;
		}
		//
		delete[] Message;
		delete messageLength;
		return true;
	}
	return true;
}
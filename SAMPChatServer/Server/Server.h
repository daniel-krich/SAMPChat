
DWORD WINAPI ServerThread(void* arg);
bool OnClientConnectHandle(ConnectedClient *client);
void OnClientDisconnectHandle(ConnectedClient *client);
void OnClientSendMessageHandle(ConnectedClient *client, const rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator>& data);
//
void OnDialogResponse(ConnectedClient* client, uint16_t dialogId, uint16_t itemId, uint8_t buttonId, std::string response);
void OnCommandResponse(ConnectedClient* client, std::string data);
//
void OnServerInit(TCPServer* server, unsigned short port);

enum SendMessageType
{
	SendClientMessage,
	ShowClientDialog
};

enum ResponseMessageType
{
	CommandResponse,
	DialogResponse
};

enum DialogResponseButton
{
	DialogDecline,
	DialogAccept
};

enum DialogStyles
{
	DIALOG_STYLE_MSGBOX,
	DIALOG_STYLE_INPUT,
	DIALOG_STYLE_LIST,
	DIALOG_STYLE_PASSWORD,
	DIALOG_STYLE_TABLIST,
	DIALOG_STYLE_TABLIST_HEADERS
};
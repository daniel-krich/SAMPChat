#define SOCKET_STATUS_READ 0x1
#define SOCKET_STATUS_WRITE 0x2
#define SOCKET_STATUS_EXCEPT 0x4

namespace TCP {

typedef void(*OnReceiveData)(const rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator>&);
typedef void(*OnConnectionClosed)();


//
class TCPClient
{
private:
	SOCKET client;
	addrinfo addr;
	std::vector<char> RecvBuffer;
public:
	uint8_t Connection;
	uint16_t CurrentDialogID;
	void Send(std::string data);
	void Close();
	TCPClient(std::string IP, unsigned int port);
	void HandleConnection(OnReceiveData ReceiveData, OnConnectionClosed ConnectionClosed);
	int GetSocketStatus(SOCKET socket, int status);
	~TCPClient();
};

void enable_keepalive(SOCKET sock);

void OnReceiveDataCall(const rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator>& data);
void OnConnectionClosedCall();
void OnConnectionFailCall();

enum ConnectionStatus
{
	Connected,
	Disconnected
};

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

};
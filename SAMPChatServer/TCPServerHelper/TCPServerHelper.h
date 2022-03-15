
#define SOCKET_STATUS_READ 0x1
#define SOCKET_STATUS_WRITE 0x2
#define SOCKET_STATUS_EXCEPT 0x4

#define SOCKET_TIMEOUT 7000

#define CHAT_RGB(r,g,b) \
    ((DWORD)((((0xff)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

class ConnectedClient;
class TCPServer;

typedef bool(*func_accept_connection)(ConnectedClient*);
typedef void(*func_process_data)(ConnectedClient*, const rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator>&);
typedef void(*func_drop_connection)(ConnectedClient*);

typedef std::function<bool(ConnectedClient*)> find_if_function;


struct FamilyStruct
{
	int id;
	std::string name;
	DWORD color;
};

struct ServerStore
{
	std::list<FamilyStruct> families;
};

class TCPServer
{
private:
	func_drop_connection dropConnectionFunc;
	SOCKET serverSocket;
	std::list<ConnectedClient*> connectedClients;
	std::vector<char> recvBuffer;
	int GetSocketStatus(SOCKET socket, int status);
	void HandleIncoming(func_accept_connection f1);
	void HandleData(func_process_data f2);
	void MySQLChecker();
	void CleanupSockets();
public:
	TCPServer(unsigned short port);
	void Initialize(func_accept_connection f1, func_process_data f2, func_drop_connection f3);
	void CloseSocket(SOCKET socket);
	void Exit();
	ServerStore storage;
	std::list<ConnectedClient*> ConnectedUsers();
	std::list<ConnectedClient*> FindUsersIf(find_if_function fif);
	MySQLConnector* SQLInstance();
	MySQLConnector* sql;
	~TCPServer();
};

struct Store
{
	//Account related
	int m_ID;
	bool m_LoggedIn;
	std::string m_Name;
	int m_FamilyID;
	int m_FamilyRank;


	//Temporary stuff
	std::string t_TempName;
	uint16_t t_CurrentDialogId;
	int t_TempUserId_Invite;
};

class ConnectedClient
{
private:
public:
	TCPServer* serverInstance;
	SOCKET socket;
	sockaddr_in addr;
	ConnectedClient(TCPServer* instance);
	void Send(std::string data);
	void SendClientMessage(std::string message, DWORD color);
	void ShowClientDialog(int dialogId, int dialogType, std::string dialogHeader, std::string dialogText, std::string dialogButton1, std::string dialogButton2 = "");
	void Close();
	Store storage;
	std::string GetIP();
	~ConnectedClient();
};
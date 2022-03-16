#include "../main.h"

TCPServer::TCPServer(unsigned short port)
{
	WSADATA wsaData;
	WORD vers = MAKEWORD(2, 2);
	if (WSAStartup(vers, &wsaData) != NO_ERROR) return;
    this->serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->serverSocket != INVALID_SOCKET)
	{
        sockaddr_in hint;
        hint.sin_family = AF_INET;
        hint.sin_port = htons(port);
		hint.sin_addr.S_un.S_addr = INADDR_ANY;
		bind(this->serverSocket, (sockaddr*)&hint, sizeof(hint));
        listen(this->serverSocket, SOMAXCONN);

        //Non blocking socket
        unsigned long argp = 1;
        setsockopt(this->serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&argp, sizeof(argp));
        ioctlsocket(this->serverSocket, FIONBIO, &argp);
        OnServerInit(this, port);
	}
}

TCPServer::~TCPServer()
{
    this->connectedClients.remove_if([](ConnectedClient* client) {
        delete client;
        return true;
    });
    delete this->sql;
    WSACleanup();
}

void TCPServer::Exit()
{
    this->serverSocket = INVALID_SOCKET;
}

MySQLConnector* TCPServer::SQLInstance()
{
    return this->sql;
}

void TCPServer::Initialize(func_accept_connection f1, func_process_data f2, func_drop_connection f3)
{
    this->dropConnectionFunc = f3;
    while (int SocketStatus = this->GetSocketStatus(this->serverSocket, SOCKET_STATUS_READ) != SOCKET_ERROR)
    {
        this->HandleIncoming(f1);
        this->HandleData(f2);
        this->CleanupSockets();
        this->MySQLChecker();
        Sleep(1);
    }
}

int TCPServer::GetSocketStatus(SOCKET socket, int status)
{
    static timeval instantSpeedPlease = { 0,0 };
    fd_set a = { 1, {socket} };
    fd_set* read = ((status & 0x1) != 0) ? &a : NULL;
    fd_set* write = ((status & 0x2) != 0) ? &a : NULL;
    fd_set* except = ((status & 0x4) != 0) ? &a : NULL;
    int result = select(0, read, write, except, &instantSpeedPlease);
    if (result == SOCKET_ERROR)
    {
        result = WSAGetLastError();
    }
    if (result < 0 || result > 3)
    {
        return SOCKET_ERROR;
    }
    return result;
}

void TCPServer::CloseSocket(SOCKET socket)
{
    std::list<ConnectedClient*>::iterator it;
    it = std::find_if(this->connectedClients.begin(), this->connectedClients.end(), [&](ConnectedClient* client) {
        if (client->socket == socket)
        {
            this->dropConnectionFunc(client);
            return true;
        }
        else return false;
    });
    if (it != this->connectedClients.end())
    {
        it._Ptr->_Myval->socket = INVALID_SOCKET;
    }
    shutdown(socket, SD_BOTH);
    closesocket(socket);
}

void TCPServer::HandleIncoming(func_accept_connection f1)
{
    int client_size = sizeof(sockaddr_in);
    SOCKET tempSocket;
    sockaddr_in tempSocket_sockaddr_in;
    tempSocket = accept(this->serverSocket, (struct sockaddr*)&tempSocket_sockaddr_in, &client_size);
    if (tempSocket != INVALID_SOCKET)
    {
        ConnectedClient* clientSocket = new ConnectedClient(this);
        clientSocket->socket = tempSocket;
        clientSocket->addr = tempSocket_sockaddr_in;
        if (f1(clientSocket) == true)
        {
            enable_keepalive(clientSocket->socket);
            this->connectedClients.push_back(clientSocket);
            return;
        }
        shutdown(clientSocket->socket, SD_BOTH);
        closesocket(clientSocket->socket);
        delete clientSocket;
    }
}

void TCPServer::HandleData(func_process_data f2)
{
    for(ConnectedClient* Client : this->connectedClients)
    {
        int SocketStatus = this->GetSocketStatus(Client->socket, SOCKET_STATUS_READ);
        if (SocketStatus == 1 && Client->socket != INVALID_SOCKET)
        {
            unsigned long BufferSize = 0;
            ioctlsocket(Client->socket, FIONREAD, &BufferSize);
            if (BufferSize > 0)
            {
                this->recvBuffer.clear();
                unsigned long BufferFreePointer = 0;
                int RecvStatus = 0;
                do {
                    BufferFreePointer = (unsigned long)this->recvBuffer.size();
                    this->recvBuffer.resize((size_t)BufferFreePointer + BufferSize, 0);
                    RecvStatus = recv(Client->socket, (char*)(&this->recvBuffer[BufferFreePointer]), BufferSize, 0);
                } while (RecvStatus > 0);
                this->recvBuffer.resize((size_t)BufferFreePointer, 0);
                //
                using JsonDocument = rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator>;
                JsonDocument Empty;
                JsonDocument Recv;
                Recv.SetObject();
                Recv.Parse(this->recvBuffer.data());
                f2(Client, Recv);
                Recv.Swap(Empty);
                //
                this->recvBuffer.clear();
                this->recvBuffer.shrink_to_fit();
            }
            else if (BufferSize <= 0) //disconnect
            {
                this->CloseSocket(Client->socket);
            }
        }
    }
}

void TCPServer::CleanupSockets()
{
    this->connectedClients.remove_if([](ConnectedClient* client)
    {
        if (client->socket == INVALID_SOCKET)
        {
            delete client;
            return true;
        }
        else return false;
    });
}

void TCPServer::MySQLChecker()
{
    if (this->SQLInstance()->ReconnectInterval <= GetTickCount64())
    {
        if (this->SQLInstance()->SqlConnection->isClosed() || !this->SQLInstance()->SqlConnection->isValid())
        {
            if (this->SQLInstance()->SqlConnection->reconnect()) PrintGUI("[SQL] Connection to MySQL was lost and restored.");
            else PrintGUI("[SQL] Connection to MySQL was lost, retrying...");
            //
            this->SQLInstance()->ReconnectInterval = GetTickCount64() + 1500;
        }
    }
}

std::list<ConnectedClient*> TCPServer::ConnectedUsers()
{
    return this->connectedClients;
}

std::list<ConnectedClient*> TCPServer::FindUsersIf(find_if_function fif)
{
    std::list<ConnectedClient*> ReturnUsers;
    for (ConnectedClient* client : this->connectedClients)
    {
        if (fif(client))
        {
            ReturnUsers.push_back(client);
        }
    }
    return ReturnUsers;
}

//
ConnectedClient::ConnectedClient(TCPServer* instance)
{
    this->serverInstance = instance;
}

ConnectedClient::~ConnectedClient()
{

}

void ConnectedClient::Send(std::string data)
{
    send(this->socket, data.c_str() + '\0', data.length() + 1, 0);
}

void ConnectedClient::SendClientMessage(std::string message, DWORD color)
{
    using JsonDocument = rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator>;
    JsonDocument Empty;
    JsonDocument SCM;
    SCM.SetObject();
    SCM.AddMember("type", SendMessageType::SendClientMessage, SCM.GetAllocator());
    SCM.AddMember("message", message, SCM.GetAllocator());
    SCM.AddMember("color", (uint64_t)color, SCM.GetAllocator());
    rapidjson::StringBuffer bufferSCM;
    rapidjson::Writer<rapidjson::StringBuffer> writer(bufferSCM);
    SCM.Accept(writer);
    send(this->socket, bufferSCM.GetString() + '\0', bufferSCM.GetLength() + 1, 0);
    SCM.Swap(Empty);
}

void ConnectedClient::ShowClientDialog(int dialogId, int dialogType, std::string dialogHeader, std::string dialogText, std::string dialogButton1, std::string dialogButton2)
{
    using JsonDocument = rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator>;
    JsonDocument Empty;
    JsonDocument SCD;
    SCD.SetObject();
    SCD.AddMember("type", SendMessageType::ShowClientDialog, SCD.GetAllocator());
    SCD.AddMember("dialogId", dialogId, SCD.GetAllocator());
    SCD.AddMember("dialogType", dialogType, SCD.GetAllocator());
    SCD.AddMember("dialogHeader", dialogHeader, SCD.GetAllocator());
    SCD.AddMember("dialogText", dialogText, SCD.GetAllocator());
    SCD.AddMember("dialogButton1", dialogButton1, SCD.GetAllocator());
    SCD.AddMember("dialogButton2", dialogButton2, SCD.GetAllocator());
    rapidjson::StringBuffer bufferSCD;
    rapidjson::Writer<rapidjson::StringBuffer> writer(bufferSCD);
    SCD.Accept(writer);
    send(this->socket, bufferSCD.GetString() + '\0', bufferSCD.GetLength() + 1, 0);
    SCD.Swap(Empty);
    //
    //Anti dialog cheat
    this->storage.t_CurrentDialogId = dialogId;
}

void ConnectedClient::Close()
{
    this->serverInstance->CloseSocket(this->socket);
}

std::string ConnectedClient::GetIP()
{
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(this->addr.sin_addr), str, INET_ADDRSTRLEN);
    return std::string(str);
}

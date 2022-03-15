#include "../main.h"


TCP::TCPClient::TCPClient(std::string IP, unsigned int port)
{
    WSADATA wsaData;
    WORD vers = MAKEWORD(2, 2);
    if (WSAStartup(vers, &wsaData) != NO_ERROR) return;
    this->client = socket(AF_INET, SOCK_STREAM, 0);
    if (this->client != INVALID_SOCKET)
    {
        sockaddr_in hint;
        hint.sin_family = AF_INET;
        hint.sin_port = htons(port);
        inet_pton(AF_INET, IP.c_str(), &hint.sin_addr);
        if (connect(this->client, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR)
        {
            OnConnectionFailCall();
            this->Close();
        }
        else
        {
            //Non blocking socket
            unsigned long argp = 1;
            setsockopt(this->client, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&argp, sizeof(argp));
            ioctlsocket(this->client, FIONBIO, &argp);
            //
            enable_keepalive(this->client);
        }
    }
}

void TCP::TCPClient::HandleConnection(OnReceiveData ReceiveData, OnConnectionClosed ConnectionClosed)
{
    if (this->client == INVALID_SOCKET) return;
    while (int SocketStatus = this->GetSocketStatus(this->client, SOCKET_STATUS_READ) != SOCKET_ERROR)
    {
        SocketStatus = this->GetSocketStatus(this->client, SOCKET_STATUS_READ);
        if (SocketStatus == 1 && this->client != INVALID_SOCKET)
        {
            unsigned long BufferSize = 0;
            ioctlsocket(this->client, FIONREAD, &BufferSize);
            if (BufferSize > 0)
            {
                this->RecvBuffer.clear();
                unsigned long BufferFreePointer = 0;
                int RecvStatus = 0;
                do {
                    BufferFreePointer = (unsigned long)this->RecvBuffer.size();
                    this->RecvBuffer.resize((size_t)BufferFreePointer + BufferSize, 0);
                    RecvStatus = recv(this->client, (char*)(&this->RecvBuffer[BufferFreePointer]), BufferSize, 0);
                } while (RecvStatus > 0);
                this->RecvBuffer.resize((size_t)BufferFreePointer, 0);
                //
                std::vector<char>::iterator startPos = this->RecvBuffer.begin(), stopPos;
                while ((stopPos = std::find(startPos, this->RecvBuffer.end(), '\0')) != this->RecvBuffer.end()) {
                    using JsonDocument = rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator>;
                    JsonDocument Empty;
                    JsonDocument Recv;
                    Recv.SetObject();
                    Recv.Parse(std::string(startPos, stopPos));
                    ReceiveData(Recv);
                    Recv.Swap(Empty);
                    startPos = stopPos + 1;
                }
                //
            }
            else if (BufferSize == 0)
            {
                ConnectionClosed();
                this->Close();
            }
        }
        this->RecvBuffer.clear();
        this->RecvBuffer.shrink_to_fit();
        Sleep(1);
    }
}

TCP::TCPClient::~TCPClient()
{
    this->Close();
}

int TCP::TCPClient::GetSocketStatus(SOCKET socket, int status)
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

void TCP::TCPClient::Send(std::string data)
{
    int State = send(this->client, data.c_str(), data.length(), 0);
    if (State == SOCKET_ERROR) this->Close();
}

void TCP::TCPClient::Close()
{
    shutdown(this->client, SD_BOTH);
    closesocket(this->client);
    this->client = INVALID_SOCKET;
    this->Connection = ConnectionStatus::Disconnected;
    //WSACleanup(); -> Causes to loose connection with the SA:MP server aswell
}

void TCP::enable_keepalive(SOCKET sock)
{
    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char*>(&yes), sizeof(int));
    int idle = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, reinterpret_cast<char*>(&idle), sizeof(int));
    int interval = 5;
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, reinterpret_cast<char*>(&interval), sizeof(int));
    int maxpkt = 3;
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, reinterpret_cast<char*>(&maxpkt), sizeof(int));
}
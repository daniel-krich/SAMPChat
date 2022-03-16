#define _CRT_SECURE_NO_WARNINGS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <list>
#include <map>
#include <time.h>
#include <thread>
#include <functional>
#include <algorithm>
#include <math.h>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <mysql/include/jdbc/mysql_connection.h>
#include <mysql/include/jdbc/cppconn/driver.h>
#include <mysql/include/jdbc/cppconn/exception.h>
#include <mysql/include/jdbc/cppconn/resultset.h>
#include <mysql/include/jdbc/cppconn/statement.h>
#include <mysql/include/jdbc/cppconn/prepared_statement.h>

#include "MySQL/MySQLConnector.h"
#include "TCPServer/TCPServer.h"
#include "TCPServer/NetHelper.h"
#include "Server/Server.h"

#define PrintGUI_MAX_LINES 200

/////=========================
#define SERVER_PORT 2230

#define SQL_HOST	"tcp://127.0.0.1:3306"
#define SQL_DB		"sampchat"
#define SQL_USER	"root"
#define SQL_PASS	""
/////=========================

LRESULT CALLBACK WindowActions(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

void GlueConsoleData(std::list<std::string> arr, std::string &out);
void PrintGUI(std::string lpString);
extern TCPServer* Server;
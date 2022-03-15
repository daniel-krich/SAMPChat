#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#pragma warning( disable : 4409 )
#pragma warning( disable : 4250 )
#pragma warning( disable : 4100 )
#pragma warning( disable : 4733 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4273 )
#pragma warning( disable : 4804 )


#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <shellapi.h>
#include <d3dx9.h>
#include <Gdiplus.h>
#include <assert.h>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <functional>
#include <Thread>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <intrin.h>
#pragma intrinsic(_ReturnAddress)


#include "./SAMPlugin/FunctionHooks.h"
#include "./TCPClient/Client.h"
extern TCP::TCPClient* client;


// typedefs/classes for legacy
#include "./SAMP/stddefs.h"
#include "./SAMP/stdtypes.h"

// selected MTA class includes
#include "./SAMP/CDirect3DData.h"

// RakNet stuff
#include "./SAMP/BitStream.h"
#include "./SAMP/RakClient.h"
#include "./SAMP/HookedRakClient.h"

// normal includes
#include "./SAMP/CDetour.h"
#include "./SAMP/math_stuff.h"
#include "./SAMP/cheat.h"
#include "./SAMP/mem_funcs.h"
#include "./SAMP/samp.h"
#include "./SAMP/proxyIDirect3D9.h"


// externals
extern HMODULE					g_hDllModule;
extern uint32_t					g_dwSAMP_Addr;
extern D3DPRESENT_PARAMETERS* g_pGTAPresent;
extern std::list<std::string> ReceivedData;

#define CHAT_RGB(r,g,b) \
    ((D3DCOLOR)((((0xff)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

void CmdCallback(PCHAR params);
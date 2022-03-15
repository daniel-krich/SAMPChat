/*

	PROJECT:		mod_sa
	LICENSE:		See LICENSE in the top level directory
	COPYRIGHT:		Copyright we_sux, BlastHack

	mod_sa is available from https://github.com/BlastHackNet/mod_s0beit_sa/

	mod_sa is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	mod_sa is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with mod_sa.  If not, see <http://www.gnu.org/licenses/>.

	*/
#include "../main.h"

#define SAMP_DLL		"samp.dll"
#define SAMP_CMP		"F8036A004050518D4C24"


//randomStuff
extern int						iViewingInfoPlayer;
int								g_iCursorEnabled = 0;
char							g_m0dCmdlist[(SAMP_MAX_CLIENTCMDS - 22)][30];
int								g_m0dCmdNum = 0;
bool							g_m0dCommands = false;

// global samp pointers
DWORD							d_Input8 = 0;
int								iIsSAMPSupported = 0;
int								g_renderSAMP_initSAMPstructs = 0;
stSAMP							*g_SAMP = nullptr;
stPlayerPool					*g_Players = nullptr;
stVehiclePool					*g_Vehicles = nullptr;
stChatInfo						*g_Chat = nullptr;
stInputInfo						*g_Input = nullptr;
stKillInfo						*g_DeathList = nullptr;
stScoreboardInfo				*g_Scoreboard = nullptr;
stDialogInfo					*g_Dialog = nullptr;

// global managed support variables
stTranslateGTASAMP_vehiclePool	translateGTASAMP_vehiclePool;
stTranslateGTASAMP_pedPool		translateGTASAMP_pedPool;

stStreamedOutPlayerInfo			g_stStreamedOutInfo;



//////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// FUNCTIONS //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// update SAMPGTA vehicle translation structure

// update SAMPGTA ped translation structure

void waitForSamp()
{
	while (1)
	{
		g_dwSAMP_Addr = getSampAddress();
		if (g_dwSAMP_Addr == NULL)
		{
			Sleep(50);
			continue;
		}
		g_SAMP = stGetSampInfo();
		g_Chat = stGetSampChatInfo();
		g_Input = stGetInputInfo();
		g_DeathList = stGetKillInfo();
		g_Scoreboard = stGetScoreboardInfo();
		g_Dialog = stGetDialogInfo();
		if (g_SAMP)
		{
			d_Input8 = (DWORD)LoadLibraryA("DINPUT8.dll");
			g_RakClient = new RakClient(g_SAMP->pRakClientInterface);
			g_SAMP->pRakClientInterface = (RakClientInterface*)new HookedRakClientInterface();
			break;
		}
		else Sleep(50);
	}
	SetupSAMPHook("HandleRPCPacket", SAMP_HOOKENTER_HANDLE_RPC, hook_handle_rpc_packet, DETOUR_TYPE_JMP, 6);
	SetupSAMPHook("HandleRPCPacket2", SAMP_HOOKENTER_HANDLE_RPC2, hook_handle_rpc_packet2, DETOUR_TYPE_JMP, 8);
	SetupSAMPHook("DialogResponse", SAMP_HOOKENTER_DIALOGRESPONSE, hook_handle_dialog_response, DETOUR_TYPE_JMP, 6);
}

uint32_t getSampAddress()
{
	uint32_t	samp_dll;
	samp_dll = (uint32_t) LoadLibraryA(SAMP_DLL);
	return samp_dll;
}

template<typename T>
T GetSAMPPtrInfo(uint32_t offset)
{
	if (g_dwSAMP_Addr == NULL)
		return NULL;
	return *(T *)(g_dwSAMP_Addr + offset);
}

struct stSAMP *stGetSampInfo(void)
{
	return GetSAMPPtrInfo<stSAMP *>(SAMP_INFO_OFFSET);
}

struct stDialogInfo *stGetDialogInfo(void)
{
	return GetSAMPPtrInfo<stDialogInfo *>(SAMP_DIALOG_INFO_OFFSET);
}

struct stChatInfo *stGetSampChatInfo(void)
{
	return GetSAMPPtrInfo<stChatInfo *>(SAMP_CHAT_INFO_OFFSET);
}

struct stInputInfo *stGetInputInfo(void)
{
	return GetSAMPPtrInfo<stInputInfo *>(SAMP_CHAT_INPUT_INFO_OFFSET);
}

struct stKillInfo *stGetKillInfo(void)
{
	return GetSAMPPtrInfo<stKillInfo *>(SAMP_KILL_INFO_OFFSET);
}

struct stScoreboardInfo *stGetScoreboardInfo(void)
{
	return GetSAMPPtrInfo<stScoreboardInfo *>(SAMP_SCOREBOARD_INFO);
}

int isBadSAMPVehicleID(int iVehicleID)
{
	if (g_Vehicles == NULL || iVehicleID == (uint16_t) -1 || iVehicleID >= SAMP_MAX_VEHICLES)
		return 1;
	return !g_Vehicles->iIsListed[iVehicleID];
}

int isBadSAMPPlayerID(int iPlayerID)
{
	if (g_Players == NULL || iPlayerID < 0 || iPlayerID > SAMP_MAX_PLAYERS)
		return 1;
	return !g_Players->iIsListed[iPlayerID];
}

void showSampDialog(int send, int dialogID, int typedialog, const char* caption, const char* text, const char* button1, const char* button2)
{
	uint32_t func = g_dwSAMP_Addr + SAMP_FUNC_DIALOG_SHOW;
	uint32_t data = g_dwSAMP_Addr + SAMP_DIALOG_INFO_OFFSET;

	__asm mov eax, dword ptr[data]
		__asm mov ecx, dword ptr[eax]
		__asm push send
	__asm push button2
	__asm push button1
	__asm push text
	__asm push caption
	__asm push typedialog
	__asm push dialogID
	__asm call func
	return;
}

bool isGameActive()
{
	if (d_Input8 == 0) return true;
	bool StateGame = *(bool*)(d_Input8 + GTA_isGameActive_OFFSET); // 0 == not active
	bool StateGamePauseMenu = *(bool*)(GTA_isPauseMenuActive);
	if (!StateGame || StateGamePauseMenu) return false;
	else return true;
}


D3DCOLOR samp_color_get(int id, DWORD trans)
{
	if (g_dwSAMP_Addr == NULL)
		return NULL;

	D3DCOLOR	*color_table;
	if (id < 0 || id >= (SAMP_MAX_PLAYERS + 3))
		return D3DCOLOR_ARGB(0xFF, 0x99, 0x99, 0x99);

	switch (id)
	{
		case (SAMP_MAX_PLAYERS) :
			return 0xFF888888;

		case (SAMP_MAX_PLAYERS + 1) :
			return 0xFF0000AA;

		case (SAMP_MAX_PLAYERS + 2) :
			return 0xFF63C0E2;
	}

	color_table = (D3DCOLOR *) ((uint8_t *) g_dwSAMP_Addr + SAMP_COLOR_OFFSET);
	return (color_table[id] >> 8) | trans;
}

int getNthPlayerID(int n)
{
	if (g_Players == NULL)
		return -1;

	int thisplayer = 0;
	for (int i = 0; i < SAMP_MAX_PLAYERS; i++)
	{
		if (g_Players->iIsListed[i] != 1)
			continue;
		if (g_Players->sLocalPlayerID == i)
			continue;
		if (thisplayer < n)
		{
			thisplayer++;
			continue;
		}

		return i;
	}

	//shouldnt happen
	return -1;
}

int getPlayerCount(void)
{
	if (g_Players == NULL)
		return NULL;

	int iCount = 0;
	int i;

	for (i = 0; i < SAMP_MAX_PLAYERS; i++)
	{
		if (g_Players->iIsListed[i] != 1)
			continue;
		iCount++;
	}

	return iCount + 1;
}

int setLocalPlayerName(const char *name)
{
	if (g_Players == NULL || g_Players->pLocalPlayer == NULL)
		return 0;
	//traceLastFunc("setLocalPlayerName()");

	int strlen_name = strlen(name);
	if (strlen_name == 0 || strlen_name > SAMP_ALLOWED_PLAYER_NAME_LENGTH)
		return 0;

	((void(__thiscall *) (void *, const char *name, int len)) (g_dwSAMP_Addr + SAMP_FUNC_NAMECHANGE)) (&g_Players->pVTBL_txtHandler, name, strlen_name);
	return 1;
}

int getVehicleCount(void)
{
	if (g_Vehicles == NULL)
		return NULL;

	int iCount = 0;
	int i;

	for (i = 0; i < SAMP_MAX_VEHICLES; i++)
	{
		if (g_Vehicles->iIsListed[i] != 1)
			continue;
		iCount++;
	}

	return iCount;
}

const char *getPlayerName(int iPlayerID)
{
	if (g_Players == NULL || iPlayerID < 0 || iPlayerID > SAMP_MAX_PLAYERS)
		return NULL;

	if (iPlayerID < 0 || iPlayerID > SAMP_MAX_PLAYERS)
		return NULL;

	if (iPlayerID == g_Players->sLocalPlayerID)
		return g_Players->strLocalPlayerName.c_str();

	if (g_Players->pRemotePlayer[iPlayerID] == NULL)
		return NULL;

	return g_Players->pRemotePlayer[iPlayerID]->strPlayerName.c_str();
}

int getPlayerState(int iPlayerID)
{
	if (g_Players == NULL || iPlayerID < 0 || iPlayerID > SAMP_MAX_PLAYERS)
		return NULL;
	if (iPlayerID == g_Players->sLocalPlayerID)
		return NULL;
	if (g_Players->iIsListed[iPlayerID] != 1)
		return NULL;
	if (g_Players->pRemotePlayer[iPlayerID]->pPlayerData == NULL)
		return NULL;

	return g_Players->pRemotePlayer[iPlayerID]->pPlayerData->bytePlayerState;
}


int getPlayerSAMPVehicleID(int iPlayerID)
{
	if (g_Players == NULL && g_Vehicles == NULL) return 0;
	if (g_Players->pRemotePlayer[iPlayerID] == NULL) return 0;
	if (g_Vehicles->pSAMP_Vehicle[g_Players->pRemotePlayer[iPlayerID]->pPlayerData->sVehicleID] == NULL) return 0;
	return g_Players->pRemotePlayer[iPlayerID]->pPlayerData->sVehicleID;
}

uint32_t getPedGTAScriptingIDFromPlayerID(int iPlayerID)
{
	if (g_Players == NULL)
		return NULL;

	if (g_Players->iIsListed[iPlayerID] != 1)
		return NULL;
	if (g_Players->pRemotePlayer[iPlayerID] == NULL)
		return NULL;
	if (g_Players->pRemotePlayer[iPlayerID]->pPlayerData == NULL)
		return NULL;
	if (g_Players->pRemotePlayer[iPlayerID]->pPlayerData->pSAMP_Actor == NULL)
		return NULL;

	return g_Players->pRemotePlayer[iPlayerID]->pPlayerData->pSAMP_Actor->ulGTAEntityHandle;
}

uint32_t getVehicleGTAScriptingIDFromVehicleID(int iVehicleID)
{
	if (g_Vehicles == NULL)
		return NULL;

	if (g_Vehicles->iIsListed[iVehicleID] != 1)
		return NULL;
	if (g_Vehicles->pSAMP_Vehicle[iVehicleID] == NULL)
		return NULL;

	return g_Vehicles->pSAMP_Vehicle[iVehicleID]->ulGTAEntityHandle;
}

void addClientCommand(const char *name, CMDPROC function)
{
	if (name == NULL || function == NULL || g_Input == NULL)
		return;

	if (g_Input->iCMDCount == (SAMP_MAX_CLIENTCMDS - 1))
	{
		//Log("Error: couldn't initialize '%s'. Maximum command amount reached.", name);
		return;
	}

	if (strlen(name) > 30)
	{
		//Log("Error: command name '%s' was too long.", name);
		return;
	}

	((void(__thiscall *) (void *_this, const char *command, CMDPROC function)) (g_dwSAMP_Addr + SAMP_FUNC_ADDCLIENTCMD)) (g_Input, name, function);
}

void SetupSAMPHook(const char* szName, DWORD dwFuncOffset, void* Func, int iType, int iSize)
{
	CDetour api;
	api.Create((uint8_t*)((uint32_t)g_dwSAMP_Addr) + dwFuncOffset, (uint8_t*)Func, iType, iSize);
}

uint8_t _declspec (naked) hook_handle_dialog_response(void)
{
	static DWORD dwJumpAddr, dwButtonID;
	__asm pushad;
	__asm mov eax, dword ptr[esp + 0x24];
	__asm mov dwButtonID, eax;
	HandleDialogResponse(g_Dialog->iDialogID, dwButtonID,
		((int(__thiscall*)(void*, int))(g_dwSAMP_Addr + SAMP_FUNC_DIALOG_GETITEM))(g_Dialog->pListBox, -1),
		((const char* (__thiscall*)(void*))(g_dwSAMP_Addr + SAMP_FUNC_DIALOG_GETTEXT))(g_Dialog->pEditBox));
	dwJumpAddr = g_dwSAMP_Addr + SAMP_HOOKEXIT_DIALOGRESPONSE;
	__asm popad;
	__asm
	{
		push ebp;
		mov ebp, esp;
		and esp, 0xFFFFFFF8;
		jmp dwJumpAddr;
	}
}

void addToChatWindow(std::string text, D3DCOLOR textColor, int playerID)
{
	if (g_SAMP == NULL || g_Chat == NULL)
		return;

	if (text.empty())
		return;

	if (playerID < -1)
		playerID = -1;

	void(__thiscall *AddToChatWindowBuffer) (void *, ChatMessageType, const char *, const char *, D3DCOLOR, D3DCOLOR) =
		(void(__thiscall *) (void *_this, ChatMessageType Type, const char *szString, const char *szPrefix, D3DCOLOR TextColor, D3DCOLOR PrefixColor))
		(g_dwSAMP_Addr + SAMP_FUNC_ADDTOCHATWND);

	if (playerID != -1)
	{
		// getPlayerName does the needed validity checks, no need for doubles
		char *playerName = (char*) getPlayerName(playerID);
		if (playerName == NULL)
			return;
		AddToChatWindowBuffer(g_Chat, CHAT_TYPE_CHAT, text.c_str(), playerName, textColor, samp_color_get(playerID));
	}
	else
	{
		AddToChatWindowBuffer(g_Chat, CHAT_TYPE_DEBUG, text.c_str(), nullptr, textColor, 0);
	}
}

void restartGame()
{
	if (g_SAMP == NULL)
		return;

	((void(__thiscall *) (void *)) (g_dwSAMP_Addr + SAMP_FUNC_RESTARTGAME)) (g_SAMP);
}

void playerSpawn(void)
{
	if (g_SAMP == NULL)
		return;

	((void(__thiscall *) (void *_this)) (g_dwSAMP_Addr + SAMP_FUNC_REQUEST_SPAWN)) (g_Players->pLocalPlayer);
	((void(__thiscall *) (void *_this)) (g_dwSAMP_Addr + SAMP_FUNC_SPAWN)) (g_Players->pLocalPlayer);
}

void disconnect(int reason /*0=timeout, 500=quit*/)
{
	if (g_SAMP == NULL)
		return;

	g_RakClient->GetInterface()->Disconnect(reason);
}

void setPassword(const char *password)
{
	if (g_SAMP == NULL)
		return;

	g_RakClient->GetInterface()->SetPassword(password);
}

void sendSetInterior(uint8_t interiorID)
{
	if (g_SAMP == NULL)
		return;

	((void(__thiscall *) (void *_this, byte interiorID)) (g_dwSAMP_Addr + SAMP_FUNC_SENDINTERIOR)) (g_Players->pLocalPlayer, interiorID);
}

void setSpecialAction(uint8_t byteSpecialAction)
{
	if (g_SAMP == NULL)
		return;

	if (g_Players->pLocalPlayer == NULL)
		return;

	((void(__thiscall *) (void *_this, byte specialActionId)) (g_dwSAMP_Addr + SAMP_FUNC_SETSPECIALACTION)) (g_Players->pLocalPlayer, byteSpecialAction);
}

void sendSCMEvent(int iEvent, int iVehicleID, int iParam1, int iParam2)
{
	g_RakClient->SendSCMEvent(iVehicleID, iEvent, iParam1, iParam2);
}

void toggleSAMPCursor(int iToggle)
{
	if (g_SAMP == NULL) return;
	if (g_Input->iInputEnabled) return;

	void		*obj = *(void **) (g_dwSAMP_Addr + SAMP_MISC_INFO);
	((void(__thiscall *) (void *, int, bool)) (g_dwSAMP_Addr + SAMP_FUNC_TOGGLECURSOR))(obj, iToggle ? 3 : 0, !iToggle);
	if (!iToggle)
		((void(__thiscall *) (void *)) (g_dwSAMP_Addr + SAMP_FUNC_CURSORUNLOCKACTORCAM))(obj);
	// g_iCursorEnabled = iToggle;
}

void sendDeath(void)
{
	if (g_SAMP == NULL)
		return;
	((void(__thiscall *) (void *)) (g_dwSAMP_Addr + SAMP_FUNC_DEATH))
		(g_Players->pLocalPlayer);
}

void updateScoreboardData(void)
{
	((void(__thiscall *) (void *_this)) (g_dwSAMP_Addr + SAMP_FUNC_UPDATESCOREBOARDDATA)) (g_SAMP);
}

void toggleOffScoreboard(void)
{
	((void(__thiscall *) (void *_this, bool hideCursor)) (g_dwSAMP_Addr + SAMP_FUNC_DISABLE_SCOREBOARD)) (g_Scoreboard, true);
}

uint8_t _declspec (naked) hook_handle_rpc_packet(void)
{
	static DWORD dwTemp1, dwTemp2;
	__asm pushad;
	__asm mov dwTemp1, eax; // RPCParameters rpcParms
	__asm mov dwTemp2, edi; // RPCNode *node

	HookRPCIN(*(unsigned char*)dwTemp2, (RPCParameters*)dwTemp1, *(void(**) (RPCParameters * rpcParams))(dwTemp2 + 1));
	dwTemp1 = g_dwSAMP_Addr + SAMP_HOOKEXIT_HANDLE_RPC;

	__asm popad;
	// execute overwritten code
	__asm add esp, 4
	// exit from the custom code
	__asm jmp dwTemp1;
}

uint8_t _declspec (naked) hook_handle_rpc_packet2(void)
{
	static DWORD dwTemp1, dwTemp2;
	__asm pushad;
	__asm mov dwTemp1, ecx; // RPCParameters rpcParms
	__asm mov dwTemp2, edi; // RPCNode *node

	HookRPCIN(*(unsigned char*)dwTemp2, (RPCParameters*)dwTemp1, *(void(**) (RPCParameters * rpcParams))(dwTemp2 + 1));
	dwTemp1 = g_dwSAMP_Addr + SAMP_HOOKEXIT_HANDLE_RPC2;

	__asm popad;
	// exit from the custom code
	__asm jmp dwTemp1;
}

void __stdcall CNetGame__destructor(void)
{
	// release hooked rakclientinterface, restore original rakclientinterface address and call CNetGame destructor
	if (g_SAMP->pRakClientInterface != NULL)
		delete g_SAMP->pRakClientInterface;
	g_SAMP->pRakClientInterface = g_RakClient->GetInterface();
	return ((void(__thiscall *) (void *)) (g_dwSAMP_Addr + SAMP_FUNC_CNETGAMEDESTRUCTOR))(g_SAMP);
}

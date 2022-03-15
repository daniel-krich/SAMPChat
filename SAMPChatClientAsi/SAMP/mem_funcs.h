/*
	PROJECT:		mod_sa
	LICENSE:		See LICENSE in the top level directory
	COPYRIGHT:		Copyright we_sux, FYP
	mod_sa is available from http://code.google.com/p/m0d-s0beit-sa/
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
#define VEHICLE_LIST_ID_START		400
#define VEHICLE_LIST_SIZE			212

#define VEHICLE_CLASS_CAR			0
#define VEHICLE_CLASS_CAR_FAST		1
#define VEHICLE_CLASS_HEAVY			2
#define VEHICLE_CLASS_HELI			3
#define VEHICLE_CLASS_AIRPLANE		4
#define VEHICLE_CLASS_BIKE			5
#define VEHICLE_CLASS_BOAT			6
#define VEHICLE_CLASS_MINI			7
#define VEHICLE_CLASS_TRAILER		8
#define VEHICLE_CLASS_COUNT			9	/* # of classes */

#define VEHICLE_UPGRADES_COUNT		194

#define ACTOR_IS_DEAD( a )			( (a)->state == ACTOR_STATE_DYING || (a)->state == ACTOR_STATE_DEAD )
#define ACTOR_ALIVE					0x01
#define ACTOR_NOT_SAME_VEHICLE		0x02

#define VEHICLE_ALIVE				0x01
#define VEHICLE_EMPTY				0x02
#define VEHICLE_OCCUPIED			0x04
#define VEHICLE_NOTBURNING			0x08

#define FUNC_GET_CAR_ID				0x424160
#define FUNC_GET_CAR_STRUCT			0x4048E0
#define FUNC_GET_ACTOR_ID			0x4442D0
#define FUNC_GET_ACTOR_STRUCT		0x404910
#define FUNC_GET_OBJECT_ID			0x465070
#define FUNC_GET_OBJECT_STRUCT		0x465040
#define FUNC_GET_COL_MODEL_STRUCT	0x40FB80
#define FUNC_GET_BUILDING_STRUCT	0x403FA0

struct weapon_entry
{
	int			id;
	int			slot;
	int			model_id;
	const char* name;
};

struct vehicle_entry
{
	int			id;			// model id
	int			class_id;	// class id
	const char* name;		// vehicle name
	int			passengers; // total passenger seats, 0-10
};

struct interiors
{
	int			interior_id;
	float		pos[3];
	float		rotation;
	const char* interior_name;
};

struct vehicle_upgrade_info
{
	int			iModelID;	// iModelID
	const char* upgradeCategoryName;	// upgrade name
	const char* upgradeName;			// upgrade name
};

struct str_split
{
	char* str;
	char** argv;
	int		argc;
};


// this blows the doors off Microsoft's version, mmmmkay
int									isBadPtr_GTA_pBuildingInfo(DWORD p_BuildingInfo);
int									isBadPtr_GTA_pObjectInfo(DWORD p_ObjectInfo);
bool								isBadPtr_handlerAny(void* pointer, ULONG size, DWORD dwFlags);
bool								isBadPtr_readAny(void* pointer, ULONG size);
bool								isBadPtr_writeAny(void* pointer, ULONG size);

// oldschool pool
uint32_t							GetFromPool(DWORD value, DWORD Pool, DWORD function);

// gta SCM stuff
extern inline int					ScriptCarId(struct vehicle_info* mecar);
extern inline int					ScriptActorId(struct actor_info* meactor);
extern inline int					ScriptObjectId(struct object_info* object);
extern inline struct vehicle_info* GetVehicleByGtaId(int car_id);
extern inline struct actor_info* GetActorByGtaId(int actor_id);
extern inline struct object_info* GetObjectByGtaId(int object_id);

// functions to get/set certain things
// this should be phased out in favor of MTA/R* classes
void								gta_weather_state_set(int state);
void								gta_time_hour_set(int hour);
void								gta_money_set(uint32_t amount);
uint32_t							gta_money_get(void);
void								gta_game_speed_set(float value);
float								gta_game_speed_get(void);
void								gta_gravity_set(float value);
float								gta_gravity_get(void);
struct checkpoint* gta_checkpoint_info_get(int n);
int									gta_menu_active(void);
void								gta_menu_active_set(int enabled);
const struct weapon_entry* gta_weapon_get_by_name(const char* name);
void								gta_weapon_set(struct actor_info* info, int slot, int id, int ammo, int ammo_clip);
int									gta_weapon_ammo_set(struct actor_info* info, int slot, int ammo);
int									gta_weapon_ammo_clip_set(struct actor_info* info, int slot, int ammo_clip);
const char* gta_vehicle_class_name(int id);
const struct vehicle_entry* gta_vehicle_get_by_id(int id);
void								gta_vehicle_spawn(int vehicle_id);
void								gta_jetpack_give(void);
int									gta_interior_id_get(void);
void								gta_interior_id_set(int id);
void								gta_blow_all_cars();
struct actor_info* actor_info_get(int id, int flags);
struct vehicle_info* vehicle_info_get(int id, int flags);

// called by TIME_GET macro
uint64_t							__time_get(void);

// m0d functions, eventually some of this will become part of the m0d framework
void								cheat_state_text(const char* fmt, ...);
void								cheat_teleport_history_save(void);

// more gta functions
void								vehicle_detachables_teleport(struct vehicle_info* info, const float from[3],
	const float to[3]);
void								cheat_vehicle_tires_set(struct vehicle_info* info, uint8_t tire_status);
int									vehicle_contains_trailer(struct vehicle_info* info,
	const struct vehicle_info* trailer);
int									vehicle_contains_actor(struct vehicle_info* vehicle, struct actor_info* actor);
void								vehicle_state_store(const struct vehicle_info* info, struct vehicle_state* state);
void								vehicle_state_restore(struct vehicle_info* info, const struct vehicle_state* state);
void								vehicle_prevent_below_height(struct vehicle_info* info, float height);
int									vehicle_find(int id, int dir, int flags);
int									actor_find(int id, int dir, int flags);
int									vehicle_filter_flags(vehicle_info* info, int flags);
int									vehicle_find_nearest(int flags);
int									samp_actor_find_nearest(int flags);
int									actor_find_nearest(int flags);
int									screen_actor_find_nearest();
struct vehicle_info* actor_vehicle_get(const struct actor_info* info);
void								SetCloudsEnabled(int iEnabled);
void								loadAllWeaponModels(void);
void								loadSpecificModel(int iModelID);

// ped animations
bool								pPedSelf_setMoveAnimation__array(int move_animations__arrayId);
bool								isMoveAnimationIdValid(int iAnimationID);
bool								pPedSelf_setMoveAnimation(int iAnimationID);

// new vehicle types and upgrades
bool								vehicle_iModelID_IsValid(int iModelID);
bool								vehicle_upgrade_iModelID_IsValid(int iModelID);
eClientVehicleType					GetVehicleType(int iModelID);
bool								isUpgradeCompatible(int iModelID, vehicle_info* vinfo);
bool								GetSlotFromUpgrade(int us, int& ucSlot);
bool								vehicle_iModelID_IsValid(int iModelID);
void								vehicle_addUpgrade(vehicle_info* vinfo, int iModelID);
vehicle_upgrade_info				get_cveh_upgrade_info(int iModelID, int& iUpgradeInfoID);
int									vehicle_getPaintJob(vehicle_info* vinfo);
void								vehicle_setPaintJob(vehicle_info* vinfo, unsigned char new_paintjob);
int									vehicle_getColor0(vehicle_info* vinfo);
int									vehicle_getColor1(vehicle_info* vinfo);
void								vehicle_setColor0(vehicle_info* vinfo, int new_color);
void								vehicle_setColor1(vehicle_info* vinfo, int new_color);

// --------------------------------------------------------------------------------------------
int									memcpy_safe(void* dest, const void* src, uint32_t len, int check = NULL, const void* checkdata = NULL);
int									memset_safe(void* _dest, int c, uint32_t len);
int									memcmp_safe(const void* _s1, const void* _s2, uint32_t len);

void* dll_baseptr_get(const char* dll_name);

void								str_split_free(struct str_split* split);
struct str_split* str_split(const char* str, const char* ch);

size_t								strlcpy(char* dst, const char* src, size_t size);
size_t								strlcat(char* dst, const char* src, size_t size);

void* memdup(const void* src, int len);
uint8_t* hex_to_bin(const char* str);
D3DCOLOR							hex_to_color(const char* str, int len);

extern const struct weapon_entry	weapon_list[];
extern const struct vehicle_entry	vehicle_list[VEHICLE_LIST_SIZE];
extern const struct interiors		interiors_list[];

// new functions making use of MTA/R* classes
int									getVehicleGTAIDFromInterface(DWORD* vehicle);
int									getPedGTAIDFromInterface(DWORD* ped);
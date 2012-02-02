/*
 * Copyright(c) 1997-2001 Id Software, Inc.
 * Copyright(c) 2002 The Quakeforge Project.
 * Copyright(c) 2006 Quake2World.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __G_TYPES_H__
#define __G_TYPES_H__

#include "quake2world.h"

// server commands sent opaquely to the client game
typedef enum {
	SV_CMD_SCORES = SV_CMD_CGAME,
	SV_CMD_FOOBAR
} g_sv_cmd_t;

// scores are transmitted as binary to the client game module
typedef struct {
	unsigned short player_num;
	unsigned short ping;
	byte team;
	byte color;
	short score;
	short captures;
} player_score_t;

#ifdef __G_LOCAL_H__

// edict->spawnflags
#define SF_ITEM_TRIGGER			0x00000001
#define SF_ITEM_NO_TOUCH		0x00000002
#define SF_ITEM_HOVER			0x00000004

// we keep these around for compatibility with legacy levels
#define SF_NOT_EASY				0x00000100
#define SF_NOT_MEDIUM			0x00000200
#define SF_NOT_HARD				0x00000400
#define SF_NOT_DEATHMATCH		0x00000800
#define SF_NOT_COOP				0x00001000

#define SF_ITEM_DROPPED			0x00010000
#define SF_ITEM_TARGETS_USED	0x00020000

// edict->flags
#define FL_FLY					0x00000001
#define FL_SWIM					0x00000002  // implied immunity to drowning
#define FL_GOD_MODE				0x00000004
#define FL_TEAM_SLAVE			0x00000008  // not the first on the team
#define FL_RESPAWN				0x80000000  // used for item respawning

// memory tags to allow dynamic memory to be cleaned up
#define TAG_GAME	765  // clear when unloading the dll
#define TAG_LEVEL	766  // clear when loading a new level

// ammo types
typedef enum {
	AMMO_NONE,
	AMMO_SHELLS,
	AMMO_BULLETS,
	AMMO_GRENADES,
	AMMO_ROUNDS,
	AMMO_KNIVES,
} g_ammo_t;

// armor types
typedef enum {
	ARMOR_NONE, ARMOR_JACKET, ARMOR_COMBAT, ARMOR_BODY, ARMOR_SHARD
} g_armor_t;

// health types
typedef enum {
	HEALTH_NONE, HEALTH_SMALL, HEALTH_MEDIUM, HEALTH_LARGE, HEALTH_MEGA
} g_health_t;

// edict->move_type values
typedef enum {
	MOVE_TYPE_NONE, // never moves
	MOVE_TYPE_NO_CLIP, // origin and angles change with no interaction
	MOVE_TYPE_PUSH, // no clip to world, push on box contact
	MOVE_TYPE_STOP, // no clip to world, stops on box contact

	MOVE_TYPE_WALK, // gravity
	MOVE_TYPE_FLY,
	MOVE_TYPE_TOSS,
// gravity
} g_move_type_t;

// a synonym for readability
#define MOVE_TYPE_THINK MOVE_TYPE_NONE

// g_item_t->flags
typedef enum {
	ITEM_WEAPON, ITEM_AMMO, ITEM_ARMOR, ITEM_FLAG, ITEM_HEALTH, ITEM_POWERUP
} g_item_type_t;

typedef struct g_item_s {
	char *class_name; // spawning name
	boolean_t (*pickup)(struct g_edict_s *ent, struct g_edict_s *other);
	void (*use)(struct g_edict_s *ent, struct g_item_s *item);
	void (*drop)(struct g_edict_s *ent, struct g_item_s *item);
	void (*weapon_think)(struct g_edict_s *ent);
	char *pickup_sound;
	char *model;
	unsigned int effects;

	// client side info
	char *icon;
	char *pickup_name; // for printing on pickup

	unsigned short quantity; // for ammo how much, for weapons how much is used per shot
	char *ammo; // for weapons
	g_item_type_t type; // g_item_type_t, see above
	unsigned short tag; // type-specific flags

	char *precaches; // string of all models, sounds, and images this item will use
} g_item_t;

// override quake2 items for legacy maps
typedef struct {
	char *old;
	char *new;
} g_override_t;

extern g_override_t g_overrides[];

// spawn_temp_t is only used to hold entity field values that
// can be set from the editor, but aren't actually present
// in g_edict_t at runtime
typedef struct {
	// world vars, we use strings to avoid ambiguity between 0 and unset
	char *sky;
	char *weather;
	char *gravity;
	char *gameplay;
	char *teams;
	char *ctf;
	char *match;
	char *rounds;
	char *frag_limit;
	char *round_limit;
	char *capture_limit;
	char *time_limit;
	char *give;
	char *music;

	int lip;
	int distance;
	int height;
	char *noise;
	float pause_time;
	char *item;
} g_spawn_temp_t;

#define SOFS(x) (ptrdiff_t)&(((g_spawn_temp_t *)0)->x)

typedef enum {
	STATE_TOP, STATE_BOTTOM, STATE_UP, STATE_DOWN
} g_move_state_t;

typedef struct {
	// fixed data
	vec3_t start_origin;
	vec3_t start_angles;
	vec3_t end_origin;
	vec3_t end_angles;

	unsigned short sound_start;
	unsigned short sound_middle;
	unsigned short sound_end;

	float accel;
	float speed;
	float decel;
	float distance;

	float wait;

	// state data
	g_move_state_t state;
	vec3_t dir;
	float current_speed;
	float move_speed;
	float next_speed;
	float remaining_distance;
	float decel_distance;
	void (*done)(g_edict_t *);
} g_move_info_t;

// this structure is left intact through an entire game
typedef struct {
	g_edict_t *edicts; // [g_max_entities]
	g_client_t *clients; // [sv_max_clients]

	g_spawn_temp_t spawn;

	unsigned short num_items;
	unsigned short num_overrides;
} g_game_t;

extern g_game_t g_game;

// this structure is cleared as each map is entered
typedef struct {
	unsigned int frame_num;
	float time;

	char title[MAX_STRING_CHARS]; // the descriptive name (Stress Fractures, etc)
	char name[MAX_QPATH]; // the server name (fractures, etc)
	int gravity; // defaults to 800
	int gameplay; // DEATHMATCH, INSTAGIB, ARENA
	int teams;
	int ctf;
	int match;
	int rounds;
	int frag_limit;
	int round_limit;
	int capture_limit;
	float time_limit;
	char give[MAX_STRING_CHARS];
	char music[MAX_STRING_CHARS];

	// intermission state
	float intermission_time; // time intermission started
	vec3_t intermission_origin;
	vec3_t intermission_angle;
	const char *changemap;

	boolean_t warmup; // shared by match and round

	boolean_t start_match;
	float match_time; // time match started
	unsigned int match_num;

	boolean_t start_round;
	float round_time; // time round started
	unsigned int round_num;

	char vote_cmd[64]; // current vote in question
	unsigned int votes[3]; // current vote tallies
	float vote_time; // time vote started

	g_edict_t *current_entity; // entity running from G_RunFrame
} g_level_t;

// means of death
#define MOD_UNKNOWN					0
#define MOD_SHOTGUN					1
#define MOD_SUPER_SHOTGUN			2
#define MOD_MACHINEGUN				3
#define MOD_GRENADE					4
#define MOD_GRENADE_SPLASH			5
#define MOD_ROCKET					6
#define MOD_ROCKET_SPLASH			7
#define MOD_HYPERBLASTER			8
#define MOD_LIGHTNING				9
#define MOD_LIGHTNING_DISCHARGE		10
#define MOD_RAILGUN					11
#define MOD_BFG_LASER				12
#define MOD_BFG_BLAST				13
#define MOD_WATER					14
#define MOD_SLIME					15
#define MOD_LAVA					16
#define MOD_CRUSH					17
#define MOD_TELEFRAG				18
#define MOD_FALLING					19
#define MOD_SUICIDE					20
#define MOD_EXPLOSIVE				21
#define MOD_TRIGGER_HURT			22
#define MOD_HELD_GRENADE                23
#define MOD_SPLASH                      24
#define MOD_G_SPLASH			25
#define MOD_HG_SPLASH			26
#define MOD_FRIENDLY_FIRE			0x8000000

#define MAX_MAP_LIST_ELTS 64
#define MAP_LIST_WEIGHT 16384

//Action Additions

#define DF_SKINTEAMS            64
#define DF_MODELTEAMS           128

#define DAMAGE_TIME             0.5
#define FALL_TIME               0.3
#define MELEE_DISTANCE  80

#define ACTION_VERSION  "TNG " VERSION
#define TNG_VERSION             "AQ2W: The Next Generation"
#define TNG_VERSION2    "AQ2W: The Next Generation " VERSION
#define MAX_LAST_KILLED				8

#define MK23_NAME    "MK23 Pistol"
#define MP5_NAME     "MP5/10 Submachinegun"
#define M4_NAME      "M4 Assault Rifle"
#define M3_NAME      "M3 Super 90 Assault Shotgun"
#define HC_NAME      "Handcannon"
#define SNIPER_NAME  "Sniper Rifle"
#define DUAL_NAME    "Dual MK23 Pistols"
#define KNIFE_NAME   "Combat Knife"
#define GRENADE_NAME "M26 Fragmentation Grenade"

#define SIL_NAME     "Silencer"
#define SLIP_NAME    "Stealth Slippers"
#define BAND_NAME    "Bandolier"
#define KEV_NAME     "Kevlar Vest"
#define HELM_NAME    "Kevlar Helmet"
#define LASER_NAME   "Lasersight"

//AQ2:TNG - Igor adding wp_flags/itm_flags
#define WPF_MK23      0x00000001
#define WPF_MP5       0x00000002
#define WPF_M4        0x00000004
#define WPF_M3        0x00000008
#define WPF_HC        0x00000010
#define WPF_SNIPER    0x00000020
#define WPF_DUAL      0x00000040
#define WPF_KNIFE     0x00000080
#define WPF_GRENADE   0x00000100

#define ITF_SIL       0x00000001
#define ITF_SLIP      0x00000002
#define ITF_BAND      0x00000004
#define ITF_KEV       0x00000008
#define ITF_LASER     0x00000010
#define ITF_HELM      0x00000020

#define NOTEAM          0
#define TEAM1           1
#define TEAM2           2
#define TEAM3           3

#define MAX_TEAMS       3
#define TEAM_TOP        (MAX_TEAMS+1)

#define WINNER_NONE     0
#define WINNER_TEAM1    1
#define WINNER_TEAM2    2
#define WINNER_TEAM3    3
#define WINNER_TIE      4



#define NO_NUM                                  -1

#define MK23_NUM                                0
#define MP5_NUM                                 1
#define M4_NUM                                  2
#define M3_NUM                                  3
#define HC_NUM                                  4
#define SNIPER_NUM                              5
#define DUAL_NUM                                6
#define KNIFE_NUM                               7
#define GRENADE_NUM                             8

#define SIL_NUM                                 9
#define SLIP_NUM                                10
#define BAND_NUM                                11
#define KEV_NUM                                 12
#define LASER_NUM                               13
#define HELM_NUM                                14

#define MK23_ANUM                               15
#define MP5_ANUM                                16
#define M4_ANUM                                 17
#define SHELL_ANUM                              18
#define SNIPER_ANUM                             19

#define FLAG_T1_NUM                             20
#define FLAG_T2_NUM                             21

#define GRAPPLE_NUM                             22
#define MAX_SKINLEN                             32
#define WEAPON_COUNT                    9
#define ITEM_COUNT                              6
#define AMMO_COUNT                              5
#define ILIST_COUNT                             WEAPON_COUNT+ITEM_COUNT+AMMO_COUNT

typedef struct itemList_s
{
        int             index;
        int             flag;
} itemList_t;

extern itemList_t items[ILIST_COUNT];
extern int tnums[ITEM_COUNT];

// types of locations that can be hit
#define LOC_HDAM 1              // head
#define LOC_CDAM 2              // chest
#define LOC_SDAM 3              // stomach
#define LOC_LDAM 4              // legs
#define LOC_KVLR_HELMET 5       // kevlar helmet        Freud, for %D
#define LOC_KVLR_VEST 6         // kevlar vest          Freud, for %D
#define LOC_NO 7                // Shot by shotgun or handcannon

// sniper modes
#define SNIPER_1X 0
#define SNIPER_2X 1
#define SNIPER_4X 2
#define SNIPER_6X 3

//TempFile sniper zoom moved to constants
#define SNIPER_FOV1     90
#define SNIPER_FOV2     45
#define SNIPER_FOV4     20
#define SNIPER_FOV6     10

#define GRENADE_IDLE_FIRST  40
#define GRENADE_IDLE_LAST   69
#define GRENADE_THROW_FIRST 4
#define GRENADE_THROW_LAST  9   // throw it on frame 8?
// these should be server variables, when I get around to it
//#define UNIQUE_WEAPONS_ALLOWED 2
//#define UNIQUE_ITEMS_ALLOWED   1
#define SPEC_WEAPON_RESPAWN 1
#define BANDAGE_TIME    27      // 10 = 1 second
#define BLEED_TIME      10      // 10 = 1 second is time for losing 1 health at slowest bleed rate
// Igor's back in Time to hard grenades :-)
//#define GRENADE_DAMRAD  170
#define GRENADE_DAMRAD  250

//AQ2:TNG - Slicer New location support
#define MAX_LOCATIONS_IN_BASE           256     // Max amount of locations
// location structure
typedef struct
{
  int x;
  int y;
  int z;
  int rx;
  int ry;
  int rz;
  char desc[128];
}
placedata_t;

// Externals for accessing location structures
extern int ml_count;
extern placedata_t locationbase[];
extern char ml_build[6];
extern char ml_creator[101];
//AQ2:TNG END

void Cmd_Ghost_f (g_edict_t *ent);
void Cmd_AutoRecord_f(g_edict_t *ent);

typedef struct team_s
{
        char name[32];
        char skin[MAX_SKINLEN];
        char skin_index[MAX_QPATH];
        int score, total;
        int ready, locked;
        int pauses_used, wantReset;
        cvar_t  *teamscore;
}team_t;

extern team_t teams[TEAM_TOP];
extern int pause_time;
#define PARSE_BUFSIZE 256


// voting
#define MAX_VOTE_TIME 60.0
#define VOTE_MAJORITY 0.51

typedef enum
{
  DAMAGE_NO,
  DAMAGE_YES,                   // will take damage if hit
  DAMAGE_AIM                    // auto targeting recognizes this
}
damage_t;

typedef enum
{
  WEAPON_READY,
  WEAPON_ACTIVATING,
  WEAPON_DROPPING,
  WEAPON_FIRING,

  WEAPON_END_MAG,
  WEAPON_RELOADING,
  WEAPON_BURSTING,
  WEAPON_BUSY,                  // used by sniper rifle when engaging zoom, if I want to make laser sight toggle on/off  this could be used for th$
  WEAPON_BANDAGING
}
weaponstate_t;

//deadflag
#define DEAD_NO                         0
#define DEAD_DYING                      1
#define DEAD_DEAD                       2
#define DEAD_RESPAWNABLE                3
//range
#define RANGE_MELEE                     0
#define RANGE_NEAR                      1
#define RANGE_MID                       2
#define RANGE_FAR                       3
//gib types
#define GIB_ORGANIC                     0
#define GIB_METALLIC                    1
// armor types
#define ARMOR_NONE                      0
#define ARMOR_JACKET                    1

// handedness values
#define RIGHT_HANDED                    0
#define LEFT_HANDED                     1
#define CENTER_HANDED                   2
// game.serverflags values
#define SFL_CROSS_TRIGGER_1             0x00000001
#define SFL_CROSS_TRIGGER_2             0x00000002
#define SFL_CROSS_TRIGGER_3             0x00000004
#define SFL_CROSS_TRIGGER_4             0x00000008
#define SFL_CROSS_TRIGGER_5             0x00000010
#define SFL_CROSS_TRIGGER_6             0x00000020
#define SFL_CROSS_TRIGGER_7             0x00000040
#define SFL_CROSS_TRIGGER_8             0x00000080
#define SFL_CROSS_TRIGGER_MASK          0x000000ff
// noise types for PlayerNoise
#define PNOISE_SELF                     0
#define PNOISE_WEAPON                   1
#define PNOISE_IMPACT                   2

// edict->movetype values
typedef enum
{
  MOVETYPE_NONE,                // never moves
  MOVETYPE_NOCLIP,              // origin and angles change with no interaction
  MOVETYPE_PUSH,                // no clip to world, push on box contact
  MOVETYPE_STOP,                // no clip to world, stops on box contact

  MOVETYPE_WALK,                // gravity
  MOVETYPE_STEP,                // gravity, special edge handling
  MOVETYPE_FLY,
  MOVETYPE_TOSS,                // gravity
  MOVETYPE_FLYMISSILE,          // extra size to monsters
  MOVETYPE_BOUNCE,
  MOVETYPE_BLOOD
}
movetype_t;

extern int sm_meat_index;
extern int snd_fry;
//extern int *took_damage;

//means of death
#define MOD_MK23                        34
#define MOD_MP5                         35
#define MOD_M4                          36
#define MOD_M3                          37
#define MOD_HC                          38
#define MOD_SNIPER                      39
#define MOD_DUAL                        40
#define MOD_KNIFE                       41
#define MOD_KNIFE_THROWN                42
#define MOD_BLEEDING                    43
#define MOD_GAS                         44
#define MOD_KICK                        45
//PG BUND
#define MOD_PUNCH                       50
#define MOD_GRAPPLE                     51

extern int meansOfDeath;
// zucc for hitlocation of death
extern int locOfDeath;
// stop an armor piercing round that hits a vest
unsigned extern int stopAP;
extern cvar_t *sv_gib;
extern cvar_t *use_rewards;
extern cvar_t *deathmatch;
extern cvar_t *coop;
unsigned extern int dmflags;
extern cvar_t *ff_afterround;

extern g_edict_t *g_edicts;

#define FOFS(x)  (int)&(((g_edict_t *)0)->x)
#define STOFS(x) (int)&(((spawn_temp_t *)0)->x)
#define LLOFS(x) (int)&(((level_locals_t *)0)->x)
#define CLOFS(x) (int)&(((gclient_t *)0)->x)
#define random()        ((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()       (2.0 * (random() - 0.5))

extern int team_game_going;
extern unsigned int team_round_going;
extern int lights_camera_action;
extern int holding_on_tie_check;
extern int team_round_countdown;
extern int timewarning;
extern int fragwarning;
//extern transparent_list_t *transparent_list;
//extern trace_t *trace_t_temp;
extern int current_round_length; // For RoundTimeLeft
extern int day_cycle_at;
extern int teamCount;
/*
extern cvar_t *maxentities;
extern cvar_t *needpass;
extern cvar_t *hostname;
extern cvar_t *teamplay;
extern cvar_t *radiolog;
extern cvar_t *motd_time;
extern cvar_t *actionmaps;
extern cvar_t *roundtimelimit;
extern cvar_t *maxteamkills;
extern cvar_t *tkbanrounds;
extern cvar_t *twbanrounds;
extern cvar_t *limchasecam;
extern cvar_t *roundlimit;
extern cvar_t *skipmotd;
extern cvar_t *nohud;
extern cvar_t *noscore;
extern cvar_t *actionversion;
extern cvar_t *use_voice;
extern cvar_t *ppl_idletime;
extern cvar_t *use_tourney;
extern cvar_t *use_3teams;
extern cvar_t *use_kickvote;
extern cvar_t *mv_public;
extern cvar_t *vk_public;
extern cvar_t *punishkills;
extern cvar_t *mapvote_waittime;
extern cvar_t *use_buggy_bandolier;
extern cvar_t *uvtime;
extern cvar_t *use_mapvote;     // enable map voting
extern cvar_t *use_scramblevote;
extern cvar_t *sv_crlf;
extern cvar_t *vrot;
extern cvar_t *rrot;
extern cvar_t *strtwpn;
extern cvar_t *llsound;
extern cvar_t *use_cvote;
extern cvar_t *new_irvision;
extern cvar_t *use_warnings;
extern cvar_t *matchmode;
extern cvar_t *darkmatch;
extern cvar_t *day_cycle;       // If darkmatch is on, this value is the nr of seconds between each interval (day, dusk, night, dawn)
extern cvar_t *hearall;         // used in match mode
extern cvar_t *mm_forceteamtalk;
extern cvar_t *mm_adminpwd;
extern cvar_t *mm_allowlock;
extern cvar_t *mm_pausecount;
extern cvar_t *mm_pausetime;
*/
extern cvar_t *teamdm;
extern cvar_t *teamdm_respawn;
extern cvar_t *respawn_effect;

extern cvar_t *item_respawnmode;

extern cvar_t *item_respawn;
extern cvar_t *weapon_respawn;
extern cvar_t *ammo_respawn;

extern cvar_t *wave_time;

//moved these to team struct -Mani
/*extern cvar_t *team1score;
extern cvar_t *team2score;
extern cvar_t *team3score;*/

extern cvar_t *use_punch;

extern cvar_t *radio_max;
extern cvar_t *radio_time;
extern cvar_t *radio_ban;
extern cvar_t *radio_repeat;
//SLIC2
extern cvar_t *radio_repeat_time;

extern cvar_t *hc_single;
extern cvar_t *wp_flags;
extern cvar_t *itm_flags;
extern cvar_t *use_classic;     // Use_classic resets weapon balance to 1.52
extern cvar_t *skill;
extern cvar_t *fraglimit;
extern cvar_t *timelimit;
extern cvar_t *capturelimit;
extern cvar_t *password;
extern cvar_t *g_select_empty;
extern cvar_t *dedicated;

extern cvar_t *filterban;
extern cvar_t *flood_msgs;
extern cvar_t *flood_persecond;
extern cvar_t *flood_waitdelay;

extern cvar_t *sv_gravity;
extern cvar_t *sv_maxvelocity;

extern cvar_t *gun_x, *gun_y, *gun_z;
extern cvar_t *sv_rollspeed;
extern cvar_t *sv_rollangle;

extern cvar_t *run_pitch;
extern cvar_t *run_roll;
extern cvar_t *bob_up;
extern cvar_t *bob_pitch;
extern cvar_t *bob_roll;

extern cvar_t *sv_cheats;
extern cvar_t *maxclients;

extern cvar_t *unique_weapons;
extern cvar_t *unique_items;
extern cvar_t *ir;              //toggles if bandolier works as infra-red sensor
extern cvar_t *knifelimit;
extern cvar_t *tgren;
extern cvar_t *allweapon;
extern cvar_t *allitem;

extern cvar_t *stats_endmap; // If on (1), show the accuracy/etc stats at the end of a map
extern cvar_t *stats_afterround; // TNG Stats, collect stats between rounds

extern cvar_t *auto_join;       // Automaticly join clients to teams they were on in last map.
extern cvar_t *auto_equip;      // Remember weapons and items for players between maps.

// TNG:Freud - new spawning system
extern cvar_t *use_oldspawns;
// TNG:Freud - ghosts
extern cvar_t *use_ghosts;

// zucc from action
extern cvar_t *sv_shelloff;
extern cvar_t *splatlimit;
extern cvar_t *bholelimit;

#define world   (&g_edicts[0])
// item spawnflags
#define ITEM_TRIGGER_SPAWN              0x00000001
#define ITEM_NO_TOUCH                   0x00000002
// 6 bits reserved for editor flags
// 8 bits used as power cube id bits for coop games
#define DROPPED_ITEM                    0x00010000
#define DROPPED_PLAYER_ITEM             0x00020000
#define ITEM_TARGETS_USED               0x00040000

/* typedef struct
{
  char *name;
  int ofs;
  fieldtype_t type;
  int flags;
}
field_t;
*/

void Cmd_Help_f (g_edict_t *ent);
void Cmd_Score_f (g_edict_t * ent);
void Cmd_CPSI_f (g_edict_t *ent);
void Cmd_VidRef_f (g_edict_t *ent);

void PrecacheItem (g_item_t *it);
void InitItems (void);
void SetItemNames (void);
g_item_t *FindItem (char *pickup_name);
g_item_t *FindItemByClassname (char *classname);
g_item_t *FindItemByNum (int num);
//#define ITEM_INDEX(x) ((x)-itemlist)
#define INV_AMMO(ent, num) ((ent)->client->persistent.inventory[items[(num)].index])
#define GET_ITEM(num) (GetItemByIndex(items[(num)].index))
g_edict_t *Drop_Item (g_edict_t *ent, g_item_t *item);
void SetRespawn (g_edict_t *ent, float delay);
void ChangeWeapon (g_edict_t *ent);
void SpawnItem (g_edict_t *ent, g_item_t *item);
void Think_Weapon (g_edict_t *ent);
int ArmorIndex (g_edict_t *ent);
int PowerArmorType (g_edict_t *ent);
g_item_t *GetItemByIndex (int index);
void Add_Ammo (g_edict_t *ent, g_item_t *item, int count);
void Touch_Item (g_edict_t *ent, g_edict_t *other, c_bsp_plane_t *plane, c_bsp_surface_t *surf);
void KillBox (g_edict_t *ent);
void G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
//g_edict_t *G_Find (g_edict_t *from, int fieldofs, char *match);
g_edict_t *findradius (g_edict_t *from, vec3_t org, float rad);
g_edict_t *G_PickTarget (char *targetname);
void G_UseTargets (g_edict_t *ent, g_edict_t *activator);
void G_SetMovedir (vec3_t angles, vec3_t movedir);

void G_InitEdict (g_edict_t *e);
g_edict_t *G_Spawn (void);
void G_FreeEdict (g_edict_t *e);

void G_TouchTriggers (g_edict_t *ent);
void G_TouchSolids (g_edict_t *ent);

char *G_CopyString (char *in);

//float *tv (float x, float y, float z);
char *vtos (vec3_t v);

float vectoyaw (vec3_t vec);
void vectoangles (vec3_t vec, vec3_t angles);

void OnSameTeam (g_edict_t *ent1, g_edict_t *ent2);
void CanDamage (g_edict_t *targ, g_edict_t *inflictor);
void T_Damage (g_edict_t *targ, g_edict_t *inflictor, g_edict_t *attacker,
               vec3_t dir, vec3_t point, vec3_t normal, int damage,
               int knockback, int dflags, int mod);
void T_RadiusDamage (g_edict_t *inflictor, g_edict_t *attacker, float damage,
                     g_edict_t *ignore, float radius, int mod);

// damage flags
#define DAMAGE_RADIUS                   0x00000001      // damage was indirect
#define DAMAGE_NO_ARMOR                 0x00000002      // armour does not protect from this damage
#define DAMAGE_ENERGY                   0x00000004      // damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK             0x00000008      // do not affect velocity, just view angles

#define DEFAULT_BULLET_HSPREAD                  300
#define DEFAULT_BULLET_VSPREAD                  500
#define DEFAULT_SHOTGUN_HSPREAD                 1000
#define DEFAULT_SHOTGUN_VSPREAD                 500
#define DEFAULT_DEATHMATCH_SHOTGUN_COUNT        12
#define DEFAULT_SHOTGUN_COUNT                   12
#define DEFAULT_SSHOTGUN_COUNT                  20

void ThrowHead (g_edict_t *self, char *gibname, int damage, int type);
void ThrowClientHead (g_edict_t *self, int damage);
void ThrowGib (g_edict_t *self, char *gibname, int damage, int type);
void BecomeExplosion1 (g_edict_t *self);

void ThrowDebris (g_edict_t *self, char *modelname, float speed,
                  vec3_t origin);
void fire_hit (g_edict_t *self, vec3_t aim, int damage, int kick);
void fire_bullet (g_edict_t *self, vec3_t start, vec3_t aimdir, int damage,
                  int kick, int hspread, int vspread, int mod);
void fire_shotgun (g_edict_t *self, vec3_t start, vec3_t aimdir, int damage,
                   int kick, int hspread, int vspread, int count, int mod);
//SLIC2 changed argument name hyper to hyperb
void fire_blaster (g_edict_t *self, vec3_t start, vec3_t aimdir, int damage,
                   int speed, int effect);
void fire_grenade (g_edict_t *self, vec3_t start, vec3_t aimdir, int damage,
                   int speed, float timer, float damage_radius);
void fire_grenade2 (g_edict_t *self, vec3_t start, vec3_t aimdir, int damage,
                    int speed, float timer, float damage_radius);
void fire_rocket (g_edict_t *self, vec3_t start, vec3_t dir, int damage,
                  int speed, float damage_radius, int radius_damage);
void fire_rail (g_edict_t *self, vec3_t start, vec3_t aimdir, int damage,
                int kick);
void fire_bfg (g_edict_t *self, vec3_t start, vec3_t dir, int damage,
               int speed, float damage_radius);

// zucc
int knife_attack (g_edict_t *self, vec3_t start, vec3_t aimdir, int damage,
                  int kick);
void knife_throw (g_edict_t *self, vec3_t start, vec3_t dir, int damage,
                  int speed);
void fire_bullet_sparks (g_edict_t *self, vec3_t start, vec3_t aimdir,
                         int damage, int kick, int hspread, int vspread,
                         int mod);
void fire_bullet_sniper (g_edict_t *self, vec3_t start, vec3_t aimdir,
                         int damage, int kick, int hspread, int vspread,
                         int mod);
void fire_grenade3 (g_edict_t *self, vec3_t start, vec3_t aimdir, int damage,
                    int speed);

void PlayerTrail_Init (void);
void PlayerTrail_Add (vec3_t spot);
void PlayerTrail_New (vec3_t spot);
g_edict_t *PlayerTrail_PickFirst (g_edict_t *self);
g_edict_t *PlayerTrail_PickNext (g_edict_t *self);
g_edict_t *PlayerTrail_LastSpot (void);

void respawn (g_edict_t *ent);
void BeginIntermission (g_edict_t *targ);
void PutClientInServer (g_edict_t *ent);
void InitClientPersistant (g_client_t *client);
void InitClientResp (g_client_t *client);
void InitBodyQue (void);
void ClientBeginServerFrame (g_edict_t *ent);

void player_pain (g_edict_t *self, g_edict_t *other, float kick, int damage);
void player_die (g_edict_t *self, g_edict_t *inflictor, g_edict_t *attacker,
                 int damage, vec3_t point);

void ServerCommand (void);
void SV_FilterPacket (char *from, int *temp);
void Kick_Client (g_edict_t *ent);
void Ban_TeamKiller (g_edict_t *ent, int rounds);

void ClientEndServerFrame (g_edict_t *ent);

void MoveClientToIntermission (g_edict_t *client);
void G_SetStats (g_edict_t *ent);
void G_SetSpectatorStats (g_edict_t *ent);
void G_CheckChaseStats (g_edict_t *ent);
void ValidateSelectedItem (g_edict_t *ent);
void DeathmatchScoreboardMessage (g_edict_t *client, g_edict_t *killer);

void PlayerNoise (g_edict_t *who, vec3_t where, int type);

void M_CheckBottom (g_edict_t *ent);
void M_walkmove (g_edict_t *ent, float yaw, float dist);
void M_MoveToGoal (g_edict_t *ent, float dist);
void M_ChangeYaw (g_edict_t *ent);
void GetChaseTarget (g_edict_t *ent);

//============================================================================

// client_t->anim_priority
#define ANIM_BASIC              0       // stand / run
#define ANIM_WAVE               1
#define ANIM_JUMP               2
#define ANIM_PAIN               3
#define ANIM_ATTACK             4
#define ANIM_DEATH              5
// in 3.20 there is #define ANIM_REVERSE 6    -FB
// zucc vwep - based on info from Hentai
#define ANIM_REVERSE            -1

#define MAX_SKINLEN                             32


//end action



typedef enum {
	VOTE_NO_OP, VOTE_YES, VOTE_NO
} g_vote_t;

// gameplay modes
typedef enum {
	DEATHMATCH, CTF, ARENA
} g_gameplay_t;

#define TEAM_CHANGE_TIME 5.0

// damage flags
#define DAMAGE_RADIUS			0x00000001  // damage was indirect
#define DAMAGE_NO_ARMOR			0x00000002  // armor does not protect from this damage
#define DAMAGE_ENERGY			0x00000004  // damage is from an energy based weapon
#define DAMAGE_BULLET			0x00000008  // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION	0x00000010  // armor and godmode have no effect
#define MAX_NET_NAME 64

//=== player ignoring ======================================================
//==========================================================================

#define IGNOREMENUTITLE "Ignoremenu"
#define PG_MAXPLAYERS 11

typedef g_edict_t *ignorelist_t[PG_MAXPLAYERS];

void Cmd_Ignoreclear_f (g_edict_t *self, char *s);
void Cmd_Ignorelist_f (g_edict_t *self, char *s);
void Cmd_Ignorenum_f (g_edict_t *self, char *s);
void Cmd_Ignore_f (g_edict_t *self, char *s);
void Cmd_IgnorePart_f (g_edict_t *self, char *s);
void _ClrIgnoresOn (g_edict_t *target);
int IsInIgnoreList (g_edict_t *source, g_edict_t *subject);
//void _IgnoreVoteSelected (g_edict_t *ent, pmenu_t *p);
void _ClearIgnoreList (g_edict_t *ent);
//==========================================================================

// teams
typedef struct {
	char name[16];
	char skin[32];
	short score;
	short captures;
	float name_time; // prevent change spamming
	float skin_time;
} g_team_t;

// client data that persists through respawns
typedef struct {
	unsigned int first_frame; // g_level.frame_num the client entered the game

	char user_info[MAX_USER_INFO_STRING];
	char net_name[MAX_NET_NAME];
	char sql_name[20];
	char skin[32];
	short score;
	short captures;
//action
        short stats_locations[10];      // All locational damage

        int stats_shots_t;            // Total nr of shots for TNG Stats
        int stats_shots_h;            // Total nr of hits for TNG Stats

        int stats_shots[100];       // Shots fired
        int stats_hits[100];                // Shots hit
        int stats_headshot[100];    // Shots in head
	int hs_streak;
	int last_damaged_part;
	char last_damaged_players[256];

	short health;
	short max_health;

	short armor;
	short max_armor;

	short inventory[MAX_ITEMS];

	// ammo capacities
	short max_shells;
	short max_bullets;
	short max_grenades;
	short max_rounds;
	short max_knives;

	g_item_t *weapon;
	g_item_t *last_weapon;

	boolean_t spectator; // client is a spectator
	boolean_t ready; // ready

	g_team_t *team; // current team (good/evil)
	g_vote_t vote; // current vote (yes/no)
	unsigned int match_num; // most recent match
	unsigned int round_num; // most recent arena round
	int color; // weapon effect colors
} g_client_persistent_t;

// this structure is cleared on each respawn
struct g_client_s {
	// known to server
	player_state_t ps; // communicated by server to clients
	unsigned int ping;

	// private to game

	g_client_persistent_t persistent;

	boolean_t show_scores; // sets layout bit mask in player state
	float scores_time; // eligible for scores when time > this

	unsigned short ammo_index;

	unsigned int buttons;
	unsigned int old_buttons;
	unsigned int latched_buttons;

	float weapon_think_time; // time when the weapon think was called
	float weapon_fire_time; // can fire when time > this
	float muzzle_flash_time; // should send muzzle flash when time > this
	g_item_t *new_weapon;

	int damage_armor; // damage absorbed by armor
	int damage_parmor; // damage absorbed by armor
	int damage_knockback; // impact damage
	int damage_blood; // damage taken out of health
	vec3_t damage_from; // origin for vector calculation
	
	vec3_t angles; // aiming direction
	vec3_t old_angles;
	vec3_t old_velocity;

	vec3_t cmd_angles; // angles sent over in the last command

	float respawn_time; // eligible for respawn when time > this
	float drown_time; // eligible for drowning damage when time > this
	float sizzle_time; // eligible for sizzle damage when time > this
	float fall_time, fall_value; // eligible for landing event when time > this
	float jump_time; // eligible for jump when time > this
	float pain_time; // eligible for pain sound when time > this
	float gasp_time; // eligible for gasp sound when time > this
	float footstep_time; // play a footstep when time > this

	float pickup_msg_time; // display message until time > this

	float chat_time; // can chat when time > this
	boolean_t muted;

	float quad_damage_time; // has quad when time < this
	float quad_attack_time; // play attack sound when time > this

	g_edict_t *chase_target; // player we are chasing

//Action additions

	int kills;
	int deaths;
	int streak;	
	
	int damage_dealt;
	
	g_item_t *item;		//tp item
	g_item_t *weapon;	//tp weapon
	g_item_t *lastweapon;

	int last_damaged_part;
  	char last_damaged_players[256];

	int stat_mode;                // Automatical Send of statistics to client
	int stat_mode_intermission;

	int team;		//tp team
	int saved_team;
  	int ctf_state;
	int ctf_capstreak;
	float ctf_lasthurtcarrier;
	float ctf_lastreturnedflag;
	float ctf_flagsince;
	float ctf_lastfraggedcarrier;

	int joined_team;              // last frame # at which the player joined a team
	int lastWave;                 //last time used wave

	g_edict_t *last_killed_target[MAX_LAST_KILLED];
	int killed_teammates;
	int idletime;
	int tourneynumber;
	g_edict_t *kickvote;
	ignorelist_t ignorelist;

	char *mapvote;
	char *cvote;
	
	int mk23_mode;                // firing mode, semi or auto
	int mp5_mode;
	int m4_mode;
	int knife_mode;
	int grenade_mode;
	int hc_mode;
	int id;                       // id command on or off
	int ir;                       // ir on or off (only matters if player has ir device, currently bandolier)
	int hs_streak; 
	int firing_style;
	int mk23_max;
	int mk23_rds;

	int dual_max;
	int dual_rds;
	int shot_max;
	int shot_rds;
	int sniper_max;
	int sniper_rds;

	int mp5_max;
	int mp5_rds;

	int m4_max;
	int m4_rds;
	int cannon_max;
  	int cannon_rds;
  	int knife_max;

  	int grenade_max;
  	int curr_weap;                // uses NAME_NUM values

  	int fired;                    // keep track of semi auto
  	int burst;                    // remember if player is bursting or not
  	int fast_reload;              // for shotgun/sniper rifle
  	int idle_weapon;              // how many frames to keep our weapon idle
  	int desired_fov;              // what fov does the player want? (via zooming)
	int desired_zoom;             //either 0, 1, 2, 4 or 6. This is set to 0 if no zooming shall be done, and is set to 0 after zooming is done.

 	int ctf_uvtime;               // AQ2:TNG - JBravo adding UVtime

  	int unique_weapon_total;
  	int unique_item_total;
  	int drop_knife;
  	int knife_sound;              // we attack several times when slashing but only want 1 sound

	int takedamage;

  	int no_sniper_display;
  	int bandaging;
  	int leg_damage;
  	int leg_dam_count;
  	int leg_noise;
  	int leghits;
  	int bleeding;                 //remaining points to bleed away
  	int bleed_remain;
  	int bleedloc;
  	vec3_t bleedloc_offset;       // location of bleeding (from origin)
  	vec3_t bleednorm;
  	vec3_t mins;
	vec3_t maxs;
  	solid_t solid;
  	int clipmask;
  	g_edict_t *owner;
  	float bleeddelay;             // how long until we bleed again

	
	int bandage_stopped;
  	int have_laser;

  	int doortoggle;               // set by player with opendoor command

  	g_edict_t *attacker;            // keep track of the last person to hit us
  	int attacker_mod;             // and how they hit us
  	int attacker_loc;             // location of the hit

  	int push_timeout;             // timeout for how long an attacker will get fall death credit

  	int jumping;

	int reload_attempts;
	int weapon_attempts;
	int chase_mode;

	int hand;
	int score;	

	int selected_item;
	int inventory[MAX_ITEMS];

	int max_bullets;
	int max_shells;
	int max_grenades;
	int max_rounds;
	int max_knives;
	
	vec3_t kick_angles;
	vec3_t kick_origin;
	float v_dmg_roll, v_dmg_pitch, v_dmg_time;
	float damage_alpha;
	float bonus_alpha;
	vec3_t v_angle;
	vec3_t damage_blend;
	vec3_t oldviewangles;
	vec3_t oldvelocity;
	
	
  //AQ2:TNG - Slicer Matchmode code
	int captain;
	int subteam;
	int admin;

	float rd_mute;
	int rd_Count;
	float rd_time;
};

struct g_edict_s {
	entity_state_t s;
	struct g_client_s *client; // NULL if not a player

	boolean_t in_use;
	int link_count;

	link_t area; // linked to a division node or leaf

	int num_clusters; // if -1, use head_node instead
	int cluster_nums[MAX_ENT_CLUSTERS];
	int head_node; // unused if num_clusters != -1
	int area_num, area_num2;

	unsigned sv_flags;
	vec3_t mins, maxs;
	vec3_t abs_mins, abs_maxs, size;
	solid_t solid;
	unsigned int clip_mask;
	g_edict_t *owner;

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!

	unsigned int spawn_flags;
	unsigned int flags; // FL_GOD_MODE, etc..

	char *class_name;
	char *model;

	g_move_type_t move_type;
	g_move_info_t move_info;

	float timestamp;

	char *target;
	char *target_name;
	char *path_target;
	char *kill_target;
	char *message;
	char *team;
	char *command;
	char *script;

	g_edict_t *target_ent;

	float speed, accel, decel;
	vec3_t move_dir;
	vec3_t pos1, pos2;

	vec3_t velocity;
	vec3_t avelocity;

	float mass;
	float gravity;

	float next_think;
	void (*pre_think)(g_edict_t *ent);
	void (*think)(g_edict_t *self);
	void (*blocked)(g_edict_t *self, g_edict_t *other); // move to move_info?
	void (*touch)(g_edict_t *self, g_edict_t *other, c_bsp_plane_t *plane,
			c_bsp_surface_t *surf);
	void (*use)(g_edict_t *self, g_edict_t *other, g_edict_t *activator);
	void (*pain)(g_edict_t *self, g_edict_t *other, int damage, int knockback);
	void (*die)(g_edict_t *self, g_edict_t *inflictor, g_edict_t *attacker,
			int damage, vec3_t point);

	float touch_time;
	float push_time;

	short health;
	short max_health;
	boolean_t dead;

	short view_height; // height above origin where eyesight is determined
	boolean_t take_damage;
	boolean_t damage_dealt;
	short dmg;
	short knockback;
	float dmg_radius;
	short sounds; // make this a spawntemp var?
	int count;

	g_edict_t *chain;
	g_edict_t *enemy;
	g_edict_t *activator;
	g_edict_t *ground_entity;
	int ground_entity_link_count;
	g_edict_t *team_chain;
	g_edict_t *team_master;
	g_edict_t *lightning;

	unsigned short noise_index;
	short attenuation;

	// timing variables
	float wait;
	float delay; // before firing targets
	float random;

	unsigned int water_type;
	unsigned int old_water_level;
	unsigned int water_level;

	int area_portal; // the area portal to toggle

	g_item_t *item; // for bonus items

	c_bsp_plane_t plane; // last touched
	c_bsp_surface_t *surf;

	vec3_t map_origin; // where the map says we spawn
};

#endif

#endif /* __G_TYPES_H__ */

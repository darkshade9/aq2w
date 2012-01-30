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
	AMMO_ROCKETS,
	AMMO_CELLS,
	AMMO_BOLTS,
	AMMO_SLUGS,
	AMMO_NUKES
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

// gitem_t->flags
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

#define FOFS(x) (ptrdiff_t)&(((g_edict_t *)0)->x)
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
#define MOD_FRIENDLY_FIRE			0x8000000

//action
#define MOD_HANDGRENADE			31
#define MOD_HG_SPLASH			32
#define MOD_HELD_GRENADE		33
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
extern int stopAP;


// action function
g_edict_t *FindEdictByClassnum (char *classname, int classnum);

void EjectBlooder (g_edict_t *self, vec3_t start, vec3_t veloc);
void EjectShell (g_edict_t *self, vec3_t start, int toggle);
void AddDecal (g_edict_t *self, c_trace_t *tr);
void AddSplat (g_edict_t *self, vec3_t point, c_trace_t *tr);

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
//AQ2:TNG End adding flags


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
#define TEAM_TOP        (MAX_TEAMS+1)
#define MAX_TEAMS	3
#define MAX_SKINLEN

void Cmd_Ghost_f (g_edict_t *ent);
void Cmd_AutoRecord_f(g_edict_t *ent);

typedef struct team_s
{
        char name[32];
//        char skin[MAX_SKINLEN];
        char skin_index[MAX_QPATH];
        int score, total;
        int ready, locked;
        int pauses_used, wantReset;
        cvar_t  *teamscore;
}team_t;

extern team_t teams[TEAM_TOP];
extern int pause_time;
#define PARSE_BUFSIZE 256



#define MAX_MAP_LIST_ELTS 64
#define MAP_LIST_WEIGHT 16384

// voting
#define MAX_VOTE_TIME 60.0
#define VOTE_MAJORITY 0.51

typedef enum {
	VOTE_NO_OP, VOTE_YES, VOTE_NO
} g_vote_t;

// gameplay modes
typedef enum {
	DEATHMATCH, INSTAGIB, ARENA
} g_gameplay_t;

#define TEAM_CHANGE_TIME 5.0

// damage flags
#define DAMAGE_RADIUS			0x00000001  // damage was indirect
#define DAMAGE_NO_ARMOR			0x00000002  // armor does not protect from this damage
#define DAMAGE_ENERGY			0x00000004  // damage is from an energy based weapon
#define DAMAGE_BULLET			0x00000008  // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION	0x00000010  // armor and godmode have no effect
#define MAX_NET_NAME 64

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

	short health;
	short max_health;

	short armor;
	short max_armor;

	short inventory[MAX_ITEMS];

	// ammo capacities
	short max_shells;
	short max_bullets;
	short max_grenades;
	short max_rockets;
	short max_cells;
	short max_bolts;
	short max_slugs;
	short max_nukes;

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
	int damage_blood; // damage taken out of health
	vec3_t damage_from; // origin for vector calculation

	vec3_t angles; // aiming direction
	vec3_t old_angles;
	vec3_t old_velocity;

	vec3_t cmd_angles; // angles sent over in the last command

	float respawn_time; // eligible for respawn when time > this
	float drown_time; // eligible for drowning damage when time > this
	float sizzle_time; // eligible for sizzle damage when time > this
	float fall_time; // eligible for landing event when time > this
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

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

#ifndef __G_MAIN_H__
#define __G_MAIN_H__

#ifdef __G_LOCAL_H__

// maplist structs
typedef struct g_map_list_elt_s {
	char name[32];
	char title[128];
	char sky[32];
	char weather[64];
	int gravity;
	int gameplay;
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
	float weight;
} g_map_list_elt_t;

typedef struct g_map_list_s {
	g_map_list_elt_t maps[MAX_MAP_LIST_ELTS];
	unsigned int count, index;

	// weighted random selection
	unsigned int weighted_index[MAP_LIST_WEIGHT];
	float total_weight;
} g_map_list_t;

extern g_map_list_t g_map_list;

extern g_level_t g_level;

extern g_import_t gi;
extern g_export_t ge;

extern unsigned short grenade_index, grenade_hit_index;
extern unsigned short rocket_index, rocket_fly_index;
extern unsigned short lightning_fly_index;
extern unsigned short quad_damage_index;

extern unsigned int means_of_death;

extern cvar_t *g_auto_join;
extern cvar_t *g_capture_limit;
extern cvar_t *g_chat_log;
extern cvar_t *g_cheats;
extern cvar_t *g_ctf;
extern cvar_t *g_frag_limit;
extern cvar_t *g_frag_log;
extern cvar_t *g_friendly_fire;
extern cvar_t *g_gameplay;
extern cvar_t *g_gravity;
extern cvar_t *g_match;
extern cvar_t *g_max_entities;
extern cvar_t *g_mysql;
extern cvar_t *g_mysql_db;
extern cvar_t *g_mysql_host;
extern cvar_t *g_mysql_password;
extern cvar_t *g_mysql_user;
extern cvar_t *g_player_projectile;
extern cvar_t *g_random_map;
extern cvar_t *g_round_limit;
extern cvar_t *g_rounds;
extern cvar_t *g_spawn_farthest;
extern cvar_t *g_teams;
extern cvar_t *g_time_limit;
extern cvar_t *g_voting;

//action

#define random()        ((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()       (2.0 * (random() - 0.5))

extern cvar_t *maxentities;
extern cvar_t *deathmatch;
extern cvar_t *coop;
extern cvar_t *dmflags;
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
extern cvar_t *ff_afterround;
extern cvar_t *use_buggy_bandolier;
extern cvar_t *uvtime;
extern cvar_t *use_mapvote;     // enable map voting
extern cvar_t *use_scramblevote;
extern cvar_t *sv_gib;
extern cvar_t *sv_crlf;
extern cvar_t *vrot;
extern cvar_t *rrot;
extern cvar_t *strtwpn;
extern cvar_t *llsound;
extern cvar_t *use_cvote;
extern cvar_t *new_irvision;
extern cvar_t *use_rewards;
extern cvar_t *use_warnings;
extern cvar_t *video_check;     //AQ2:TNG - Slicer: For Video Checking
extern cvar_t *video_checktime; //interval between cheat checks
extern cvar_t *video_max_3dfx;
extern cvar_t *video_max_3dfxam;
extern cvar_t *video_max_opengl;
extern cvar_t *video_check_lockpvs;
extern cvar_t *video_check_glclear;
extern cvar_t *video_force_restart;
extern cvar_t *check_time;
extern cvar_t *matchmode;
extern cvar_t *darkmatch;
extern cvar_t *day_cycle;       // If darkmatch is on, this value is the nr of seconds between each interval (day, dusk, night, dawn)

extern cvar_t *hearall;         // used in match mode
extern cvar_t *mm_forceteamtalk;
extern cvar_t *mm_adminpwd;
extern cvar_t *mm_allowlock;
extern cvar_t *mm_pausecount;
extern cvar_t *mm_pausetime;

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


extern cvar_t *password;

extern cvar_t *sv_max_clients;
extern cvar_t *dedicated;

// end action

extern g_team_t g_team_good, g_team_evil;

// text file logging
extern FILE *frag_log, *chat_log;

void G_Init(void);
void G_Shutdown(void);
void G_ResetTeams(void);
void G_ResetVote(void);
g_export_t *G_LoadGame(g_import_t *import);

#endif /* __G_LOCAL_H__ */

#endif /* __G_MAIN_H__ */


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

#include "g_local.h"

/*
 * G_OnSameTeam
 *
 * Returns true if ent1 and ent2 are on the same qmass mod team.
 */
boolean_t G_OnSameTeam(g_edict_t *ent1, g_edict_t *ent2) {

	if (!g_level.teams && !g_level.ctf)
		return false;

	if (!ent1->client || !ent2->client)
		return false;

	return ent1->client->persistent.team == ent2->client->persistent.team;
}

/*
 * G_CanDamage
 *
 * Returns true if the inflictor can directly damage the target.  Used for
 * explosions and melee attacks.
 */
boolean_t G_CanDamage(g_edict_t *targ, g_edict_t *inflictor) {
	vec3_t dest;
	c_trace_t trace;

	// bmodels need special checking because their origin is 0,0,0
	if (targ->move_type == MOVE_TYPE_PUSH) {
		VectorAdd(targ->abs_mins, targ->abs_maxs, dest);
		VectorScale(dest, 0.5, dest);
		trace = gi.Trace(inflictor->s.origin, vec3_origin, vec3_origin, dest,
				inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
		if (trace.ent == targ)
			return true;
		return false;
	}

	trace = gi.Trace(inflictor->s.origin, vec3_origin, vec3_origin,
			targ->s.origin, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy(targ->s.origin, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	trace = gi.Trace(inflictor->s.origin, vec3_origin, vec3_origin, dest,
			inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy(targ->s.origin, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
	trace = gi.Trace(inflictor->s.origin, vec3_origin, vec3_origin, dest,
			inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy(targ->s.origin, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
	trace = gi.Trace(inflictor->s.origin, vec3_origin, vec3_origin, dest,
			inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy(targ->s.origin, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	trace = gi.Trace(inflictor->s.origin, vec3_origin, vec3_origin, dest,
			inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	return false;
}

/*
 * G_Killed
 */
static void G_Killed(g_edict_t *targ, g_edict_t *inflictor,
		g_edict_t *attacker, int damage, vec3_t point) {

	if (targ->health < -999)
		targ->health = -999;

	targ->enemy = attacker;

	if (targ->move_type == MOVE_TYPE_PUSH || targ->move_type == MOVE_TYPE_STOP
			|| targ->move_type == MOVE_TYPE_NONE) { // doors, triggers, etc
		targ->die(targ, inflictor, attacker, damage, point);
		return;
	}

	targ->die(targ, inflictor, attacker, damage, point);
}

/*
 * G_SpawnDamage
 */
static void G_SpawnDamage(int type, vec3_t origin, vec3_t normal, int damage) {

	gi.WriteByte(SV_CMD_TEMP_ENTITY);
	gi.WriteByte(type);
	gi.WritePosition(origin);
	gi.WriteDir(normal);
	gi.Multicast(origin, MULTICAST_PVS);
}

/*
 * G_CheckArmor
 */
static int G_CheckArmor(g_edict_t *ent, vec3_t point, vec3_t normal,
		int damage, int dflags) {
	g_client_t *client;
	int saved;

	if (damage < 1)
		return 0;

	if (damage < 2) // sometimes protect very small damage
		damage = rand() & 1;
	else
		damage *= 0.80; // mostly protect large damage

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	if (damage > ent->client->persistent.armor)
		saved = ent->client->persistent.armor;
	else
		saved = damage;

	ent->client->persistent.armor -= saved;

	G_SpawnDamage(TE_BLOOD, point, normal, saved);

	return saved;
}

#define QUAD_DAMAGE_FACTOR 2.5
#define QUAD_KNOCKBACK_FACTOR 2.0

/*
 * G_Damage
 *
 * targ		entity that is being damaged
 * inflictor	entity that is causing the damage
 * attacker	entity that caused the inflictor to damage targ
 * 	example: targ=player2, inflictor=rocket, attacker=player1
 *
 * dir			direction of the attack
 * point       point at which the damage is being inflicted
 * normal		normal vector from that point
 * damage		amount of damage being inflicted
 * knockback	force to be applied against targ as a result of the damage
 *
 * dflags		these flags are used to control how G_Damage works
 * 	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
 * 	DAMAGE_NO_ARMOR			armor does not protect from this damage
 * 	DAMAGE_ENERGY			damage is from an energy based weapon
 * 	DAMAGE_BULLET			damage is from a bullet (used for ricochets)
 * 	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
 */
void G_Damage(g_edict_t *targ, g_edict_t *inflictor, g_edict_t *attacker,
		vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback,
		int dflags, int mod) {

	g_client_t *client;
	int take;
	int save;
	int asave;
	int te_sparks;
	float scale;

	if (!targ->take_damage)
		return;

	if (!inflictor) // use world
		inflictor = &g_game.edicts[0];

	if (!attacker) // use world
		attacker = &g_game.edicts[0];

	// quad damage affects both damage and knockback
	if (attacker->client
			&& attacker->client->persistent.inventory[quad_damage_index]) {
		damage = (int) (damage * QUAD_DAMAGE_FACTOR);
		knockback = (int) (knockback * QUAD_KNOCKBACK_FACTOR);
	}

	// friendly fire avoidance
	if (targ != attacker && (g_level.teams || g_level.ctf)) {
		if (G_OnSameTeam(targ, attacker)) { // target and attacker are on same team

			if (mod == MOD_TELEFRAG) { // telefrags can not be avoided
				mod |= MOD_FRIENDLY_FIRE;
			} else { // while everything else can
				if (g_friendly_fire->value)
					mod |= MOD_FRIENDLY_FIRE;
				else
					damage = 0;
			}
		}
	}

	// there is no self damage in instagib or arena, but there is knockback
	if (targ == attacker && g_level.gameplay)
		damage = 0;

	means_of_death = mod;

	client = targ->client;

	VectorNormalize(dir);

	// calculate velocity change due to knockback
	if (knockback && (targ->move_type != MOVE_TYPE_NONE) && (targ->move_type
			!= MOVE_TYPE_TOSS) && (targ->move_type != MOVE_TYPE_PUSH)
			&& (targ->move_type != MOVE_TYPE_STOP)) {

		vec3_t kvel;
		float mass;

		if (targ->mass < 50.0)
			mass = 50.0;
		else
			mass = targ->mass;

		scale = 1000.0; // default knockback scale

		if (targ == attacker) { // weapon jump hacks
			if (mod == MOD_BFG_BLAST)
				scale = 300.0;
			else if (mod == MOD_ROCKET_SPLASH)
				scale = 1400.0;
			else if (mod == MOD_GRENADE)
				scale = 1200.0;
		}

		VectorScale(dir, scale * (float)knockback / mass, kvel);
		VectorAdd(targ->velocity, kvel, targ->velocity);
	}

	take = damage;
	save = 0;

	// check for godmode
	if ((targ->flags & FL_GOD_MODE) && !(dflags & DAMAGE_NO_PROTECTION)) {
		take = 0;
		save = damage;
		G_SpawnDamage(TE_BLOOD, point, normal, save);
	}

	asave = G_CheckArmor(targ, point, normal, take, dflags);
	take -= asave;

	// treat cheat savings the same as armor
	asave += save;

	// do the damage
	if (take) {
		if (client)
			G_SpawnDamage(TE_BLOOD, point, normal, take);
		else {
			// impact effects for things we can hurt which shouldn't bleed
			if (dflags & DAMAGE_BULLET)
				te_sparks = TE_BULLET;
			else
				te_sparks = TE_SPARKS;

			G_SpawnDamage(te_sparks, point, normal, take);
		}

		targ->health = targ->health - take;

		if (targ->health <= 0) {
			G_Killed(targ, inflictor, attacker, take, point);
			return;
		}
	}

	if (client) {
		if (!(targ->flags & FL_GOD_MODE) && take)
			targ->pain(targ, attacker, take, knockback);
	} else if (take) {
		if (targ->pain)
			targ->pain(targ, attacker, take, knockback);
	}

	// add to the damage inflicted on a player this frame
	if (client) {
		client->damage_armor += asave;
		client->damage_blood += take;
		VectorCopy(point, client->damage_from);
	}
}

/*
 * G_RadiusDamage
 */
void G_RadiusDamage(g_edict_t *inflictor, g_edict_t *attacker,
		g_edict_t *ignore, int damage, int knockback, float radius, int mod) {
	g_edict_t *ent;
	float d, k, dist;
	vec3_t dir;

	ent = NULL;

	while ((ent = G_FindRadius(ent, inflictor->s.origin, radius)) != NULL) {

		if (ent == ignore)
			continue;

		if (!ent->take_damage)
			continue;

		VectorSubtract(ent->s.origin, inflictor->s.origin, dir);
		dist = VectorLength(dir);

		d = damage - 0.5 * dist;
		k = knockback - 0.5 * dist;

		if (d <= 0 && k <= 0) // too far away to be damaged
			continue;

		if (ent == attacker) { // reduce self damage
			if (mod == MOD_BFG_BLAST)
				d = d * 0.15;
			else
				d = d * 0.5;
		}

		if (!G_CanDamage(ent, inflictor))
			continue;

		G_Damage(ent, inflictor, attacker, dir, ent->s.origin, vec3_origin,
				(int) d, (int) k, DAMAGE_RADIUS, mod);
	}
}

//Action

qboolean CheckTeamDamage (g_edict_t *targ, g_edict_t *attacker);
{        
return false;
}

void BloodSprayThink (g_edict_t * self)
{


  /*  if ( self->dmg > 0 )
     {
     self->dmg -= 10;
     //              SpawnDamage (TE_BLOOD, self->s.origin, self->movedir, self->dmg);
     gi.WriteByte (svc_temp_entity);
     gi.WriteByte (TE_SPLASH);
     gi.WriteByte (6);
     gi.WritePosition (self->s.origin);
     gi.WriteDir (self->movedir);
     gi.WriteByte (6);       //blood
     gi.multicast (self->s.origin, MULTICAST_PVS);

     }
     else
     {
     self->think = G_FreeEdict;
     }

     self->nextthink = level.time + 0.1;
     gi.linkentity (self);
   */

  G_FreeEdict (self);
}
void blood_spray_touch (g_edict_t * ent, g_edict_t * other, c_bsp_plane_t * plane,
                   c_bsp_surface_t * surf)
{
        if (other == ent->owner)
                return;
        ent->think = G_FreeEdict;
        ent->nextthink = level.time + 0.1;
}
void blood_spray_touch (g_edict_t * ent, g_edict_t * other, c_bsp_plane_t * plane,
                   c_bsp_surface_t * surf)
{
        if (other == ent->owner)
                return;
        ent->think = G_FreeEdict;
        ent->nextthink = level.time + 0.1;
}

void spray_blood (g_edict_t * self, vec3_t start, vec3_t dir, int damage, int mod)
{
        g_edict_t *blood;
        int speed;

        switch (mod)
        {
        case MOD_MK23:
                speed = 1800;
                break;
        case MOD_MP5:
                speed = 1500;
                break;
        case MOD_M4:
                speed = 2400;
                break;
        case MOD_KNIFE:
                speed = 0;
                break;
        case MOD_KNIFE_THROWN:
                speed = 0;
                break;
        case MOD_DUAL:
                speed = 1800;
                break;
        case MOD_SNIPER:
                speed = 4000;
        default:
                speed = 1800;
        }
        blood = G_Spawn ();
        VectorNormalize (dir);
        VectorCopy (start, blood->s.origin);
        VectorCopy (dir, blood->movedir);
        vectoangles (dir, blood->s.angles);
        VectorScale (dir, speed, blood->velocity);
        blood->movetype = MOVE_TYPE_BLOOD;
        blood->clipmask = MASK_SHOT;
        blood->solid = SOLID_BBOX;
        blood->s.effects |= EF_GIB;
        VectorClear (blood->mins);
        VectorClear (blood->maxs);
        blood->s.modelindex = gi.modelindex ("sprites/null.sp2");
        blood->owner = self;
        blood->nextthink = level.time + speed / 1000;   //3.2;
        blood->touch = blood_spray_touch;
        blood->think = BloodSprayThink;
        blood->dmg = damage;
        blood->classname = "blood_spray";

        gi.linkentity (blood);
}

// zucc based on some code in Action Quake
void spray_sniper_blood (g_edict_t * self, vec3_t start, vec3_t dir)
{
        vec3_t forward;
        int mod = MOD_SNIPER;

        VectorCopy (dir, forward);

        forward[2] += .03f;

        spray_blood (self, start, forward, 0, mod);


        VectorCopy (dir, forward);
        forward[2] -= .03f;
        spray_blood (self, start, forward, 0, mod);


        VectorCopy (dir, forward);
        if ((forward[0] > 0) && (forward[1] > 0))
        {
                forward[0] -= .03f;
                forward[1] += .03f;
        }
        if ((forward[0] > 0) && (forward[1] < 0))
        {
                forward[0] += .03f;
                forward[1] += .03f;
        }
        if ((forward[0] < 0) && (forward[1] > 0))
        {
                forward[0] -= .03f;
                forward[1] -= .03f;
        }
        if ((forward[0] < 0) && (forward[1] < 0))
        {
                forward[0] += .03f;
                forward[1] -= .03f;
        }
        spray_blood (self, start, forward, 0, mod);


        VectorCopy (dir, forward);
        if ((forward[0] > 0) && (forward[1] > 0))
        {
                forward[0] += .03f;
                forward[1] -= .03f;
        }
        if ((forward[0] > 0) && (forward[1] < 0))
        {
                forward[0] -= .03f;
                forward[1] -= .03f;
        }
        if ((forward[0] < 0) && (forward[1] > 0))
        {
                forward[0] += .03f;
                forward[1] += .03f;
        }
        if ((forward[0] < 0) && (forward[1] < 0))
        {
                forward[0] -= .03f;
                forward[1] += .03f;
        }
        spray_blood (self, start, forward, 0, mod);

        VectorCopy (dir, forward);
        spray_blood (self, start, forward, 0, mod);

}


void VerifyHeadShot (vec3_t point, vec3_t dir, float height, vec3_t newpoint)
{
        vec3_t normdir;
        vec3_t normdir2;


        VectorNormalize2 (dir, normdir);
        VectorScale (normdir, height, normdir2);
        VectorAdd (point, normdir2, newpoint);
}
// zucc adding location hit code
// location hit code based off ideas by pyromage and shockman

#define LEG_DAMAGE (height/2.2) - abs(targ->mins[2]) - 3
#define STOMACH_DAMAGE (height/1.8) - abs(targ->mins[2])
#define CHEST_DAMAGE (height/1.4) - abs(targ->mins[2])

#define HEAD_HEIGHT 12.0
qboolean IsFemale (g_edict_t * ent);


void
T_Damage (g_edict_t * targ, g_edict_t * inflictor, g_edict_t * attacker, vec3_t dir,
          vec3_t point, vec3_t normal, int damage, int knockback, int dflags,
          int mod)
{
        gclient_t *client;
        char buf[256];
        int take, save;
        int asave, psave;
        int te_sparks, do_sparks = 0;
        int damage_type = 0;            // used for MOD later
        int bleeding = 0;               // damage causes bleeding
        int head_success = 0;
        int instant_dam = 1;
        float z_rel;
        int height;
        float from_top;
        vec_t dist;
        float targ_maxs2;               //FB 6/1/99

        // do this before teamplay check
        if (!targ->takedamage)
                return;
        //FIREBLADE
        if (teamplay->value && mod != MOD_TELEFRAG)
        {
                if (lights_camera_action)
                        return;

                // AQ2:TNG - JBravo adding UVtime
                if (ctf->value && targ->client)
                {
                        if(targ->client->ctf_uvtime > 0)
                                return;
                        if (attacker->client && attacker->client->ctf_uvtime > 0)
                                return;
                }

                // AQ2:TNG - JBravo adding FF after rounds
                if (targ != attacker && targ->client && attacker->client &&
                        targ->client->resp.team == attacker->client->resp.team &&
                        ((int)(dmflags->value) & (DF_NO_FRIENDLY_FIRE))) {
                                if (team_round_going)
                                        return;
                                else if (!ff_afterround->value)
                                        return;
                }
                // AQ:TNG
        }
        //FIREBLADE


        // damage reduction for shotgun
        // if far away, reduce it to original action levels
        if (mod == MOD_M3)
        {
                dist = Distance(targ->s.origin, inflictor->s.origin);
                if (dist > 450.0)
                        damage = damage - 2;
        }

        targ_maxs2 = targ->maxs[2];
        if (targ_maxs2 == 4)
                targ_maxs2 = CROUCHING_MAXS2;   //FB 6/1/99

        height = abs (targ->mins[2]) + targ_maxs2;

        // locational damage code
        // base damage is head shot damage, so all the scaling is downwards
        if (targ->client)
        {
                if (!((targ != attacker) &&
                ((deathmatch->value && ((int)dmflags->value
                & (DF_MODELTEAMS | DF_SKINTEAMS)))
                || coop->value) && (attacker && attacker->client
                && OnSameTeam (targ, attacker) &&
                ((int)dmflags->value & DF_NO_FRIENDLY_FIRE)
                && (team_round_going && ff_afterround->value))))

                {

                        // TNG Stats - Add +1 to hit, make sure that hc and m3 are handles differently

                        if ((attacker->client) && (mod != MOD_M3) && (mod != MOD_HC)) {
                                strcpy(attacker->client->resp.last_damaged_players, targ->client->persistent.netname);

                                if (!teamplay->value || team_round_going || stats_afterround->value) {
                                        attacker->client->resp.stats_hits[mod]++;
                                        attacker->client->resp.stats_shots_h++;
                                }
                        }

                        // TNG Stats END


                        if (mod == MOD_MK23 || mod == MOD_MP5 || mod == MOD_M4 ||
                                mod == MOD_SNIPER || mod == MOD_DUAL || mod == MOD_KNIFE ||
                                mod == MOD_KNIFE_THROWN)
                        {
                                z_rel = point[2] - targ->s.origin[2];
                                from_top = targ_maxs2 - z_rel;
                                if (from_top < 0.0)     //FB 6/1/99
                                from_top = 0.0; //Slightly negative values were being handled wrong
                                bleeding = 1;
                                instant_dam = 0;

                                // damage reduction for longer range pistol shots
                                if (mod == MOD_MK23 || mod == MOD_DUAL)
                                {
                                        dist = Distance(targ->s.origin, inflictor->s.origin);
                                        if (dist > 600.0 && dist < 1400.0)
                                                damage = (int) (damage * 2 / 3);
                                        else if (dist > 1400.0)
                                                damage = (int) (damage * 1 / 2);
                                }
				 if (from_top < 2 * HEAD_HEIGHT)
                                {
                                        vec3_t new_point;
                                        VerifyHeadShot (point, dir, HEAD_HEIGHT, new_point);
                                        VectorSubtract (new_point, targ->s.origin, new_point);

                                        if ((targ_maxs2 - new_point[2]) < HEAD_HEIGHT
                                                && (abs (new_point[1])) < HEAD_HEIGHT * .8
                                                && (abs (new_point[0])) < HEAD_HEIGHT * .8)
                                        {
                                                head_success = 1;
                                        }
                                }

                              if (head_success)
                                {
                                        if (attacker->client)
                                        {
                                                if (!teamplay->value || team_round_going || stats_afterround->value) {
                                                        attacker->client->resp.stats_headshot[mod]++;
                                                }
                                                //AQ2:TNG Slicer Last Damage Location
                                                if (INV_AMMO(targ, HELM_NUM)) {
                                                        attacker->client->resp.last_damaged_part = LOC_KVLR_HELMET;
                                                        if ((!teamplay->value || team_round_going || stats_afterround->value))
                                                                attacker->client->resp.stats_locations[LOC_KVLR_HELMET]++;
                                                } else {
                                                        attacker->client->resp.last_damaged_part = LOC_HDAM;
                                                        if ((!teamplay->value || team_round_going || stats_afterround->value))
                                                                attacker->client->resp.stats_locations[LOC_HDAM]++;
                                                }

                                                //AQ2:TNG END
                                                if (!OnSameTeam (targ, attacker))
                                                        attacker->client->resp.hs_streak++;

                                                // AQ:TNG Igor[Rock] changing sound dir
                                                if (attacker->client->resp.hs_streak == 3)
                                                {
                                                        if (use_rewards->value)
                                                        {
                                                                sprintf (buf, "ACCURACY %s!", attacker->client->persistent.netname);
                                                                CenterPrintAll (buf);
                                                                gi.sound (&g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD,
                                                                        gi.soundindex ("tng/accuracy.wav"), 1.0, ATTN_NONE, 0.0);
                                                        }
                                                        attacker->client->resp.hs_streak = 0;
                                                }
                                                // end of changing sound dir
                                        }

                                        if (INV_AMMO(targ, HELM_NUM) && mod != MOD_KNIFE
                                                && mod != MOD_KNIFE_THROWN && mod != MOD_SNIPER)
                                        {
                                                if (attacker->client)
                                                {
                                                        gi.cprintf (attacker, PRINT_HIGH, "%s has a Kevlar Helmet - AIM FOR THE BODY!\n",
                                                                targ->client->persistent.netname);
                                                        gi.cprintf (targ, PRINT_HIGH, "Kevlar Helmet absorbed a part of %s's shot\n",
                                                                attacker->client->persistent.netname);
                                                }
                                                gi.sound (targ, CHAN_ITEM, gi.soundindex("misc/vest.wav"), 1,
                                                        ATTN_NORM, 0);
                                                damage = (int) (damage / 2);
                                                damage_type = LOC_HDAM;
                                                bleeding = 0;
                                                instant_dam = 1;
                                                stopAP = 1;
                                                do_sparks = 1;
                                        }
                                        else if (INV_AMMO(targ, HELM_NUM) && mod == MOD_SNIPER)
                                        {
                                                if (attacker->client)
                                                {
                                                        gi.cprintf (attacker, PRINT_HIGH,
                                                        "%s has a Kevlar Helmet, too bad you have AP rounds...\n",
                                                        targ->client->persistent.netname);
                                                        gi.cprintf (targ, PRINT_HIGH,
                                                        "Kevlar Helmet absorbed some of %s's AP sniper round\n",
                                                        attacker->client->persistent.netname);
                                                }
                                                damage = (int) (damage * 0.325);
                                                gi.sound (targ, CHAN_VOICE, gi.soundindex("misc/headshot.wav"), 1,
                                                        ATTN_NORM, 0);
                                                damage_type = LOC_HDAM;
                                        }
                                        else
                                        {
                                                damage = damage * 1.8 + 1;
                                                gi.cprintf (targ, PRINT_HIGH, "Head damage\n");
                                                if (attacker->client)
                                                        gi.cprintf (attacker, PRINT_HIGH, "You hit %s in the head\n",
                                                                targ->client->persistent.netname);
                                                damage_type = LOC_HDAM;
                                                if (mod != MOD_KNIFE && mod != MOD_KNIFE_THROWN)
                                                        gi.sound (targ, CHAN_VOICE, gi.soundindex ("misc/headshot.wav"), 1,
                                                                ATTN_NORM, 0);
                                        //else
                                        //      gi.sound(targ, CHAN_VOICE, gi.soundindex("misc/glurp.wav"), 1, ATTN_NORM, 0);
                                        }
                                }
                                else if (z_rel < LEG_DAMAGE)
                                {
                                        damage = damage * .25;
                                        gi.cprintf (targ, PRINT_HIGH, "Leg damage\n");
                                        if (attacker->client)
                                        {
                                                attacker->client->resp.hs_streak = 0;
                                                gi.cprintf (attacker, PRINT_HIGH, "You hit %s in the legs\n",
                                                        targ->client->persistent.netname);
                                        }
                                        damage_type = LOC_LDAM;
                                        targ->client->leg_damage = 1;
                                        targ->client->leghits++;
                                        //AQ2:TNG Slicer Last Damage Location
                                        attacker->client->resp.last_damaged_part = LOC_LDAM;
                                        //AQ2:TNG END
                                        if (!teamplay->value || team_round_going || stats_afterround->value)
                                                attacker->client->resp.stats_locations[LOC_LDAM]++; // TNG Stats
                                }
                                else if (z_rel < STOMACH_DAMAGE)
                                {
                                        damage = damage * .4;
                                        gi.cprintf (targ, PRINT_HIGH, "Stomach damage\n");
                                        if (attacker->client)
                                        {
                                                attacker->client->resp.hs_streak = 0;
                                                gi.cprintf (attacker, PRINT_HIGH, "You hit %s in the stomach\n",
                                                        targ->client->persistent.netname);
                                        }
                                        damage_type = LOC_SDAM;
                                        //TempFile bloody gibbing
                                        if (mod == MOD_SNIPER && sv_gib->value)
                                                ThrowGib (targ, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
                                        //AQ2:TNG Slicer Last Damage Location
                                        attacker->client->resp.last_damaged_part = LOC_SDAM;
                                        //AQ2:TNG END
                                        if (!teamplay->value || team_round_going || stats_afterround->value)
                                                attacker->client->resp.stats_locations[LOC_SDAM]++; // TNG Stats
                                }
                                else            //(z_rel < CHEST_DAMAGE)
                                {
					if (attacker->client)
                                        {
                                                attacker->client->resp.hs_streak = 0;
                                        }

                                        if (INV_AMMO(targ, KEV_NUM) && mod != MOD_KNIFE
                                                && mod != MOD_KNIFE_THROWN && mod != MOD_SNIPER)
                                        {
                                                if (attacker->client)
                                                {
                                                        gi.cprintf (attacker, PRINT_HIGH, "%s has a Kevlar Vest - AIM FOR THE HEAD!\n",
                                                                targ->client->persistent.netname);
                                                        gi.cprintf (targ, PRINT_HIGH, "Kevlar Vest absorbed most of %s's shot\n",
                                                                attacker->client->persistent.netname);
                                                        /*
                                                        if (IsFemale(targ))
                                                        gi.cprintf(attacker, PRINT_HIGH, "You bruised %s through her Kevlar Vest\n", targ->cli$
                                                        else
                                                        gi.cprintf(attacker, PRINT_HIGH, "You bruised %s through his Kevlar Vest\n", targ->cli$
                                                        */
                                                }
                                                gi.sound (targ, CHAN_ITEM, gi.soundindex ("misc/vest.wav"), 1,
                                                        ATTN_NORM, 0);
                                                damage = (int) (damage / 10);
                                                damage_type = LOC_CDAM;
                                                bleeding = 0;
                                                instant_dam = 1;
                                                stopAP = 1;
                                                do_sparks = 1;

                                        }
                                        else if (INV_AMMO(targ, KEV_NUM) && mod == MOD_SNIPER)
                                        {
                                                if (attacker->client)
                                                {
                                                        gi.cprintf (attacker, PRINT_HIGH, "%s has a Kevlar Vest, too bad you have AP rounds...\n",
                                                                targ->client->persistent.netname);
                                                        gi.cprintf (targ, PRINT_HIGH, "Kevlar Vest absorbed some of %s's AP sniper round\n",
                                                                attacker->client->persistent.netname);
                                                }
                                                damage = damage * .325;
                                                damage_type = LOC_CDAM;
                                        }
                                        else
                                        {
                                                damage = damage * .65;
                                                gi.cprintf (targ, PRINT_HIGH, "Chest damage\n");
                                                if (attacker->client)
                                                        gi.cprintf (attacker, PRINT_HIGH, "You hit %s in the chest\n",
                                                                targ->client->persistent.netname);
                                                damage_type = LOC_CDAM;
                                                //TempFile bloody gibbing
                                                if (mod == MOD_SNIPER && sv_gib->value)
                                                        ThrowGib (targ, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
                                        }
                                        //AQ2:TNG Slicer Last Damage Location
                                        if (INV_AMMO(targ, KEV_NUM) && mod != MOD_KNIFE && mod != MOD_KNIFE_THROWN) {
                                                attacker->client->resp.last_damaged_part = LOC_KVLR_VEST;
                                                if (!teamplay->value || team_round_going || stats_afterround->value)
                                                        attacker->client->resp.stats_locations[LOC_KVLR_VEST]++; // TNG Stats
                                        } else {
                                                attacker->client->resp.last_damaged_part = LOC_CDAM;
                                                if (!teamplay->value || team_round_going || stats_afterround->value)
                                                        attacker->client->resp.stats_locations[LOC_CDAM]++; // TNG Stats
                                        }
                                        //AQ2:TNG END

                                }
              /*else
                 {

                 // no mod to damage
                 gi.cprintf(targ, PRINT_HIGH, "Head damage\n");
                 if (attacker->client)
                 gi.cprintf(attacker, PRINT_HIGH, "You hit %s in the head\n", targ->client->persistent.netname);
                 damage_type = LOC_HDAM;
                 gi.sound(targ, CHAN_VOICE, gi.soundindex("misc/headshot.wav"), 1, ATTN_NORM, 0);
                 } */
                        }
                        if (team_round_going && attacker->client && targ != attacker
                                && OnSameTeam (targ, attacker))
                        {
                                Add_TeamWound (attacker, targ, mod);
                        }
                }
    }


        if (damage_type && !instant_dam)        // bullets but not vest hits
        {
                vec3_t temp;
                vec3_t temporig;
                //vec3_t forward;
                VectorMA (targ->s.origin, 50, dir, temp);
                //AngleVectors (attacker->client->v_angle, forward, NULL, NULL);
                VectorScale (dir, 20, temp);
                VectorAdd (point, temp, temporig);
                if (mod != MOD_SNIPER)
                        spray_blood (targ, temporig, dir, damage, mod);
                else
                        spray_sniper_blood (targ, temporig, dir);
        }
        if (mod == MOD_FALLING && !(targ->flags & FL_GOD_MODE) )
        {
                if (targ->client && targ->health > 0)
                {
                        gi.cprintf (targ, PRINT_HIGH, "Leg damage\n");
                        targ->client->leg_damage = 1;
                        targ->client->leghits++;
                //      bleeding = 1; for testing
                }
        }


        // friendly fire avoidance
        // if enabled you can't hurt teammates (but you can hurt yourself)
        // knockback still occurs
        if (targ != attacker &&
                ((deathmatch->value && ((int)dmflags->value & (DF_MODELTEAMS | DF_SKINTEAMS)))
                || coop->value))
        {
                if (OnSameTeam (targ, attacker))
                {
                        if ((int)dmflags->value & DF_NO_FRIENDLY_FIRE &&
                                (team_round_going || !ff_afterround->value))
                                damage = 0;
                        else
                                mod |= MOD_FRIENDLY_FIRE;
                }
        }

        meansOfDeath = mod;
        locOfDeath = damage_type;       // location

        client = targ->client;

        if (dflags & DAMAGE_BULLET)
                te_sparks = TE_SPARKS;

        VectorNormalize (dir);
        // team damage avoidance
        if (!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage (targ, attacker))
                return;
        if ((mod == MOD_M3) || (mod == MOD_HC)
        || (mod == MOD_HELD_GRENADE) || (mod == MOD_HG_SPLASH)
        || (mod == MOD_G_SPLASH)
        {
                //FB 6/3/99 - shotgun damage report stuff
                int playernum = targ - g_edicts;
                playernum--;
                if (playernum >= 0 && playernum <= game.maxclients - 1)
                        *(took_damage + playernum) = 1;
                //FB 6/3/99

                bleeding = 1;
                instant_dam = 0;
        }

  /*        if ( (mod == MOD_M3) || (mod == MOD_HC) )
     {
     instant_dam = 1;
     remain = take % 2;
     take = (int)(take/2); // balances out difference in how action and axshun handle damage/bleeding

     }
   */
        if (ctf->value)
                CTFCheckHurtCarrier (targ, attacker);

        // do the damage
        if (take)
        {
                // zucc added check for stopAP, if it hit a vest we want sparks
                if (((targ->svflags) || (client)) && !do_sparks)
                        SpawnDamage (TE_BLOOD, point, normal, take);
                else
                        SpawnDamage (te_sparks, point, normal, take);

                // all things that have at least some instantaneous damage, i.e. bruising/falling
                if (instant_dam)
                        targ->health = targ->health - take;

                if (targ->health <= 0)
                {
                        if (client && attacker->client)
                        {
                                //Added these here also, if this is the last shot and before shots is from
                                //different attacker, msg's would go to wrong client -M
                                if (!OnSameTeam (attacker, targ))
                                        attacker->client->resp.damage_dealt += damage;

                                client->attacker = attacker;
                                client->attacker_mod = mod;
                                client->attacker_loc = damage_type;
                        }

                        if ((targ->svflags & SVF_MONSTER) || (client))
                                targ->flags |= FL_NO_KNOCKBACK;
                        Killed (targ, inflictor, attacker, take, point);
                        return;
                }
        }

        if (targ->svflags & SVF_MONSTER)
        {
                M_ReactToDamage (targ, attacker);
                if (!(targ->monsterinfo.aiflags & AI_DUCKED) && (take))
                {
                        targ->pain (targ, attacker, knockback, take);
                        // nightmare mode monsters don't go into pain frames often
                        if (skill->value == 3)
                                targ->pain_debounce_time = level.time + 5;
                }
        }
        else if (client)
        {
                if (!(targ->flags & FL_GOD_MODE) && (take))
                        targ->pain (targ, attacker, knockback, take);
        }
        else if (take)
        {
                if (targ->pain)
                        targ->pain (targ, attacker, knockback, take);
        }

        // add to the damage inflicted on a player this frame
        // the total will be turned into screen blends and view angle kicks
        // at the end of the frame
        if (client)
        {
		client->damage_parmor += psave;
                client->damage_armor += asave;
                client->damage_blood += take;
                client->damage_knockback += knockback;
                //zucc handle adding bleeding here
                if (damage_type && bleeding)    // one of the hit location weapons
                {
                        /* zucc add in partial bleeding, changed
                        if ( client->bleeding < 4*damage*BLEED_TIME )
                        {
                        client->bleeding = 4*damage*BLEED_TIME + client->bleeding/2;

                        }
                        else
                        {
                        client->bleeding += damage*BLEED_TIME*2;

                        } */
                        client->bleeding += damage * BLEED_TIME;
                        VectorSubtract (point, targ->absmax, targ->client->bleedloc_offset);
                        //VectorSubtract(point, targ->s.origin,  client->bleedloc_offset);

                }
                else if (bleeding)
                {
                        /*
                        if ( client->bleeding < damage*BLEED_TIME )
                        {
                        client->bleeding = damage*BLEED_TIME;
                        //client->bleedcount = 0;
                        } */
                        client->bleeding += damage * BLEED_TIME;
                        VectorSubtract (point, targ->absmax, targ->client->bleedloc_offset);
                        //VectorSubtract(point, targ->s.origin,  client->bleedloc_offset);


               }
                if (attacker->client)
                {
                        if (!OnSameTeam (attacker, targ))
                                attacker->client->resp.damage_dealt += damage;

                        client->attacker = attacker;
                        client->attacker_mod = mod;
                        client->attacker_loc = damage_type;
                        client->push_timeout = 50;
                        //VectorCopy(dir, client->bleeddir );
                        //VectorCopy(point, client->bleedpoint );
                        //VectorCopy(normal, client->bleednormal);

                }
                VectorCopy (point, client->damage_from);
        }
}


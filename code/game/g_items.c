/*
=======================================================================================================================================
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of Spearmint Source Code.

Spearmint Source Code is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

Spearmint Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Spearmint Source Code.
If not, see <http://www.gnu.org/licenses/>.

In addition, Spearmint Source Code is also subject to certain additional terms. You should have received a copy of these additional
terms immediately following the terms and conditions of the GNU General Public License. If not, please request a copy in writing from
id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
=======================================================================================================================================
*/

#include "g_local.h"

/*
  Items are any object that a player can touch to gain some effect.

  Pickup will return the number of seconds until they should respawn.

  All items should pop when dropped in lava or slime.

  Respawnable items don't actually go away when picked up, they are just made invisible and untouchable. This allows them to ride
  movers and respawn appropriately.
*/

#define RESPAWN_SP -1
#define RESPAWN_KEY 4
#define RESPAWN_ARMOR 25
#define RESPAWN_TEAM_WEAPON 30
#define RESPAWN_HEALTH 35
#define RESPAWN_AMMO 40
#define RESPAWN_HOLDABLE 60
#define RESPAWN_MEGAHEALTH 120
#define RESPAWN_POWERUP 120
#define RESPAWN_PARTIAL 998 // for multi-stage ammo/health
#define RESPAWN_PARTIAL_DONE 999 // for multi-stage ammo/health

/*
=======================================================================================================================================
Pickup_Powerup
=======================================================================================================================================
*/
int Pickup_Powerup(gentity_t *ent, gentity_t *other) {
	int quantity;

	if (!other->client->ps.powerups[ent->item->giTag]) {
		// some powerups are time based on how long the powerup is /used/ rather than timed from when the player picks it up.
		if (ent->item->giTag == PW_NOFATIGUE) {
		} else {
			// round timing to seconds to make multiple powerup timers count in sync
			other->client->ps.powerups[ent->item->giTag] = level.time - (level.time % 1000);
		}
	}
	// if an amount was specified in the ent, use it
	if (ent->count) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

	other->client->ps.powerups[ent->item->giTag] += quantity * 1000;
	// brandy also gives a little health (10)
	if (ent->item->giTag == PW_NOFATIGUE) {
		if (Q_stricmp(ent->item->classname, "item_stamina_brandy") == 0) {
			other->health += 10;

			if (other->health > other->client->ps.stats[STAT_MAX_HEALTH]) {
				other->health = other->client->ps.stats[STAT_MAX_HEALTH];
			}

			other->client->ps.stats[STAT_HEALTH] = other->health;
		}
		// cap stamina
		if (other->client->ps.powerups[PW_NOFATIGUE] > 60000) {
			other->client->ps.powerups[PW_NOFATIGUE] = 60000;
		}
	}

	if (ent->s.density == 2) { // multi-stage health first stage
		return RESPAWN_PARTIAL;
	} else if (ent->s.density == 1) { // last stage, leave the plate
		return RESPAWN_PARTIAL_DONE;
	}

	if (!(ent->spawnflags & FL_RESPAWN_SP)) {
		return RESPAWN_SP;
	}

	return RESPAWN_POWERUP;
}

/*
=======================================================================================================================================
Pickup_Key
=======================================================================================================================================
*/
int Pickup_Key(gentity_t *ent, gentity_t *other) {

	other->client->ps.stats[STAT_KEYS] |= (1 << ent->item->giTag);

	if (!(ent->spawnflags & FL_RESPAWN_SP)) {
		return RESPAWN_SP;
	}

	return RESPAWN_KEY;
}

/*
=======================================================================================================================================
Pickup_Clipboard
=======================================================================================================================================
*/
int Pickup_Clipboard(gentity_t *ent, gentity_t *other) {

	if (ent->spawnflags & FL_LEAVE_IN_WORLD) {
		return 0; // leave in world
	}

	return -1;
}

/*
=======================================================================================================================================
Pickup_Treasure
=======================================================================================================================================
*/
int Pickup_Treasure(gentity_t *ent, gentity_t *other) {

	level.numTreasureFound++;

	G_SendMissionStats();

	if (g_gametype.integer == GT_COOP) {
		other->client->ps.persistant[PERS_SCORE] += 120;
	}
#ifdef MONEY
	if (g_gametype.integer == GT_COOP_BATTLE) {
		other->client->ps.persistant[PERS_SCORE] += 400;
	}
#endif
	return RESPAWN_SP; // no respawn
}

/*
=======================================================================================================================================
UseHoldableItem

Server side handling of holdable item use.
=======================================================================================================================================
*/
void UseHoldableItem(gentity_t *ent, int item) {

	switch (item) {
		case HI_WINE: // 1921 Chateu Lafite - gives 25 pts health up to max health
			ent->health += 25;

			if (ent->health > ent->client->ps.stats[STAT_MAX_HEALTH]) {
				ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
			}

			break;
		case HI_STAMINA: // restores fatigue bar and sets "nofatigue" for a time period (currently forced to 60 sec.)
			// NOTE: currently only gives free nofatigue time, doesn't reset fatigue bar (this is because I'd like the restore to be visually gradual (on the HUD item representing current status of your fatigue) rather than snapping back to 'full')
			ent->client->ps.powerups[PW_NOFATIGUE] = 60000;
			break;
		case HI_BOOK1:
		case HI_BOOK2:
		case HI_BOOK3:
			G_AddEvent(ent, EV_POPUP_BOOK, (item - HI_BOOK1) + 1);
			break;
	}
}

/*
=======================================================================================================================================
Pickup_Holdable
=======================================================================================================================================
*/
int Pickup_Holdable(gentity_t *ent, gentity_t *other) {
	gitem_t *item;

//	item = BG_FindItemForHoldable(ent->item);
	item = ent->item;

	if (item->gameskillnumber[0]) { // if the item specifies an amount, give it
		other->client->ps.holdable[item->giTag] += item->gameskillnumber[0];
	} else {
		other->client->ps.holdable[item->giTag] += 1; // add default of 1
	}

	other->client->ps.holding = item->giTag;
	other->client->ps.stats[STAT_HOLDABLE_ITEM] |= (1 << ent->item->giTag);

	if (!(ent->spawnflags & FL_RESPAWN_SP)) {
		return RESPAWN_SP;
	}

	return RESPAWN_HOLDABLE;
}

/*
=======================================================================================================================================
Fill_Clip

Push reserve ammo into available space in the clip.
=======================================================================================================================================
*/
void Fill_Clip(playerState_t *ps, int weapon) {
	int inclip, maxclip, ammomove;
	int ammoweap = BG_FindAmmoForWeapon(weapon);

	if (weapon < WP_LUGER || weapon >= WP_NUM_WEAPONS) {
		return;
	}

	if (g_dmflags.integer & DF_NO_WEAPRELOAD) {
		return;
	}

	inclip = ps->ammoclip[BG_FindClipForWeapon(weapon)];
	maxclip = ammoTable[weapon].maxclip;
	ammomove = maxclip - inclip; // max amount that can be moved into the clip
	// cap move amount if it's more than you've got in reserve
	if (ammomove > ps->ammo[ammoweap]) {
		ammomove = ps->ammo[ammoweap];
	}

	if (ammomove) {
		if (!ps->aiChar || ps->ammo[ammoweap] < 999) { // dont take ammo away if they need unlimited supplies
			ps->ammo[ammoweap] -= ammomove;
		}

		ps->ammoclip[BG_FindClipForWeapon(weapon)] += ammomove;
	}
}

/*
=======================================================================================================================================
Add_Ammo

Try to always add ammo here unless you have specific needs (like the AI "infinite ammo" where they get below 900 and force back up to
999). fillClip will push the ammo straight through into the clip and leave the rest in reserve.
=======================================================================================================================================
*/
void Add_Ammo(gentity_t *ent, int weapon, int count, qboolean fillClip) {
	int ammoweap = BG_FindAmmoForWeapon(weapon);
	qboolean noPack = qfalse; // no extra ammo in your 'pack'

	ent->client->ps.ammo[ammoweap] += count;

	switch (ammoweap) {
		// some weaps load straight into the 'clip' since they have no storage outside the clip
		case WP_GRENADE_LAUNCHER: // make sure if he picks up a grenade that he get's the "launcher" too
		case WP_GRENADE_PINEAPPLE:
		case WP_DYNAMITE:
			COM_BitSet(ent->client->ps.weapons, ammoweap);
		case WP_TESLA:
		case WP_FLAMETHROWER:
			noPack = qtrue;
			break;
		default:
			break;
	}

	if (fillClip || noPack) {
		Fill_Clip(&ent->client->ps, weapon);
	}

	if (ent->aiCharacter) {
		noPack = qfalse; // let AI's deal with their own clip/ammo handling

	}
	// cap to max ammo
	if (noPack) {
		ent->client->ps.ammo[ammoweap] = 0;
	} else {
		if (ent->client->ps.ammo[ammoweap] > ammoTable[ammoweap].maxammo) {
			ent->client->ps.ammo[ammoweap] = ammoTable[ammoweap].maxammo;
		}

		if (count >= 999) { // 'really, give /all/'
			ent->client->ps.ammo[ammoweap] = count;
		}
	}

	if (ent->client->ps.ammoclip[ammoweap] > ammoTable[ammoweap].maxclip) {
		ent->client->ps.ammoclip[ammoweap] = ammoTable[ammoweap].maxclip;
	}

}
#ifdef MONEY
int G_GetWeaponPrice(int weapon);
int G_GetAmmoPrice(int weapon);
#endif
/*
=======================================================================================================================================
Pickup_Ammo
=======================================================================================================================================
*/
int Pickup_Ammo(gentity_t *ent, gentity_t *other) {
	int quantity;
#ifdef MONEY
	if (g_gametype.integer == GT_COOP_BATTLE) {
		other->client->ps.persistant[PERS_SCORE] += G_GetAmmoPrice(ent->item->giTag);
		return RESPAWN_SP;
	}
#endif
	if (ent->count) {
		quantity = ent->count;
	} else {
		// quantity = ent->item->quantity;
		quantity = ent->item->gameskillnumber[g_gameskill.integer];
		// FIXME: just for now
		if (!quantity) {
			quantity = ent->item->quantity;
		}
	}

	Add_Ammo(other, ent->item->giTag, quantity, qfalse);

	if (!(ent->spawnflags & FL_RESPAWN_SP)) {
		return RESPAWN_SP;
	}

	return RESPAWN_AMMO;
}

/*
=======================================================================================================================================
Pickup_Weapon
=======================================================================================================================================
*/
int Pickup_Weapon(gentity_t *ent, gentity_t *other) {
	int quantity = 0;
	qboolean alreadyHave = qfalse;
	int weapon;

	weapon = ent->item->giTag;
#ifdef MONEY
	if (g_gametype.integer == GT_COOP_BATTLE) {
		other->client->ps.persistant[PERS_SCORE] += G_GetWeaponPrice(weapon);
		return RESPAWN_SP;
	}
#endif
	if (ent->count < 0) {
		quantity = 0; // None for you, sir!
	} else {
		if (ent->count) {
			quantity = ent->count;
		} else {
			if (ent->item->quantity != -1) { // fretn
				quantity = (random() * (ammoTable[weapon].maxclip - 4)) + 4; // giving 4 - <item default count>
			}
		}
	}
	// check for special colt->akimbo add(if you've got a colt already, add the second now)
	if (weapon == WP_COLT) {
		if (COM_BitCheck(other->client->ps.weapons, WP_COLT)) {
			weapon = WP_AKIMBO;
		}
	}
	// check if player already had the weapon
	alreadyHave = COM_BitCheck(other->client->ps.weapons, weapon);
	// add the weapon
	COM_BitSet(other->client->ps.weapons, weapon);
	// throw knives
	if (g_throwKnives.integer) {
		other->client->ps.ammo[BG_FindAmmoForWeapon(WP_KNIFE)]++;
	}
	// snooper/garand
	if (weapon == WP_SNOOPERSCOPE) {
		COM_BitSet(other->client->ps.weapons, WP_GARAND);
	} else if (weapon == WP_GARAND) {
		COM_BitSet(other->client->ps.weapons, WP_SNOOPERSCOPE);
	// fg42/scope
	} else if (weapon == WP_FG42SCOPE) {
		COM_BitSet(other->client->ps.weapons, WP_FG42);
	} else if (weapon == WP_FG42) {
		COM_BitSet(other->client->ps.weapons, WP_FG42SCOPE);
	} else if (weapon == WP_SNIPERRIFLE) {
		COM_BitSet(other->client->ps.weapons, WP_MAUSER);
	}

	Add_Ammo(other, weapon, quantity, !alreadyHave);

	if (!(ent->spawnflags & FL_RESPAWN_SP)) {
		return RESPAWN_SP;
	}

	return g_weaponRespawn.integer;
}

/*
=======================================================================================================================================
Pickup_Health
=======================================================================================================================================
*/
int Pickup_Health(gentity_t *ent, gentity_t *other) {
	int max;
	int quantity = 0;

	// small and mega healths will go over the max
	if (ent->item->quantity != 5 && ent->item->quantity != 100) {
		max = other->client->ps.stats[STAT_MAX_HEALTH];
	} else {
		max = other->client->ps.stats[STAT_MAX_HEALTH] * 2;
	}

	if (ent->count) {
		quantity = ent->count;
	} else {
		if (ent->s.density) { // multi-stage health
			if (ent->s.density == 2) { // first stage (it counts down)
				quantity = ent->item->gameskillnumber[g_gameskill.integer];
			} else if (ent->s.density == 1) { // second stage
				quantity = ent->item->quantity;
			}
		} else {
			quantity = ent->item->gameskillnumber[g_gameskill.integer];
		}
	}

	other->health += quantity;

	if (other->health > max) {
		other->health = max;
	}

	other->client->ps.stats[STAT_HEALTH] = other->health;

	if (ent->s.density == 2) { // multi-stage health first stage
		return RESPAWN_PARTIAL;
	} else if (ent->s.density == 1) { // last stage, leave the plate
		return RESPAWN_PARTIAL_DONE;
	}

	return RESPAWN_SP;
}

/*
=======================================================================================================================================
Pickup_Armor
=======================================================================================================================================
*/
int Pickup_Armor(gentity_t *ent, gentity_t *other) {

	other->client->ps.stats[STAT_ARMOR] += ent->item->quantity;

	if (other->client->ps.stats[STAT_ARMOR] > 100) {
		other->client->ps.stats[STAT_ARMOR] = 100;
	}

	return RESPAWN_SP;
}

/*
=======================================================================================================================================
RespawnItem
=======================================================================================================================================
*/
void RespawnItem(gentity_t *ent) {

	if (!ent) {
		return;
	}
	// randomly select from teamed entities
	if (ent->team) {
		gentity_t *master;
		int count;
		int choice;

		if (!ent->teammaster) {
			G_Error("RespawnItem: bad teammaster");
		}

		master = ent->teammaster;

		for (count = 0, ent = master; ent; ent = ent->teamchain, count++);

		choice = rand() % count;

		for (count = 0, ent = master; ent && count < choice; ent = ent->teamchain, count++);
	}

	if (!ent) {
		return;
	}

	ent->r.contents = CONTENTS_TRIGGER;
	//ent->s.eFlags &= ~EF_NODRAW;
	ent->flags &= ~FL_NODRAW;
	ent->r.svFlags &= ~SVF_NOCLIENT;

	trap_LinkEntity(ent);
	// play the normal respawn sound only to nearby clients
	G_AddEvent(ent, EV_ITEM_RESPAWN, 0);

	ent->nextthink = 0;
}

/*
=======================================================================================================================================
Touch_Item

	if other->client->pers.autoActivate == PICKUP_ACTIVATE	(0), he will pick up items only when using +activate
	if other->client->pers.autoActivate == PICKUP_TOUCH		(1), he will pickup items when touched
	if other->client->pers.autoActivate == PICKUP_FORCE		(2), he will pickup the next item when touched(and reset to PICKUP_ACTIVATE when done)
=======================================================================================================================================
*/
void Touch_Item_Auto(gentity_t *ent, gentity_t *other, trace_t *trace) {

	if (other->client->pers.autoActivate == PICKUP_ACTIVATE) {
		return;
	}

	ent->active = qtrue;

	Touch_Item(ent, other, trace);

	if (other->client->pers.autoActivate == PICKUP_FORCE) { // autoactivate probably forced by the "Cmd_Activate_f()" function so reset it
		other->client->pers.autoActivate = PICKUP_ACTIVATE;
	}
}

/*
=======================================================================================================================================
Touch_Item
=======================================================================================================================================
*/
void Touch_Item(gentity_t *entity, gentity_t *activator, trace_t *trace) {
	int respawn;
	int makenoise = EV_ITEM_PICKUP;

	// only activated items can be picked up
	if (!entity->active) {
		return;
	} else {
		// need to set active to false if player is maxed out
		entity->active = qfalse;
	}

	if (!activator->client) {
		return;
	}

	if (G_IsClientDead(activator)) {
		return; // dead people can't pickup
	}
	// the same pickup rules are used for client side and server side
	if (!BG_CanItemBeGrabbed(&entity->s, &activator->client->ps)) {
		return;
	}
	// logging for fun stats
	activator->client->sess.pickups++;
	// call the item-specific pickup function
	switch (entity->item->giType) {
		case IT_WEAPON:
			respawn = Pickup_Weapon(entity, activator);
#ifdef MONEY
			if (g_gametype.integer == GT_COOP_BATTLE) {
				entity->wait = -1;
			}
#endif
			break;
		case IT_AMMO:
			respawn = Pickup_Ammo(entity, activator);
#ifdef MONEY
			if (g_gametype.integer == GT_COOP_BATTLE) {
				entity->wait = -1;
			}
#endif
			break;
		case IT_ARMOR:
			respawn = Pickup_Armor(entity, activator);
			break;
		case IT_HEALTH:
			respawn = Pickup_Health(entity, activator);
			break;
		case IT_POWERUP:
			respawn = Pickup_Powerup(entity, activator);
			break;
		case IT_TEAM:
			respawn = Pickup_Team(entity, activator);
			break;
		case IT_HOLDABLE:
			respawn = Pickup_Holdable(entity, activator);
			break;
		case IT_KEY:
			respawn = Pickup_Key(entity, activator);
			break;
		case IT_TREASURE:
			respawn = Pickup_Treasure(entity, activator);
			break;
		case IT_CLIPBOARD:
			respawn = Pickup_Clipboard(entity, activator);
			// send the event to the client to request that the UI draw a popup (specified by the configstring in ent->s.density)
			G_AddEvent(activator, EV_POPUP_CLIPBOARD, entity->s.density);

			if (entity->key) {
				G_AddEvent(activator, EV_GIVEPAGE, entity->key);
			}

			break;
		default:
			return;
	}

	if (!respawn) {
		return;
	}
	// play sounds
	if (entity->noise_index) {
		// a sound was specified in the entity, so play that sound (this G_AddEvent) and send the pickup as "EV_ITEM_PICKUP_QUIET"
		// so it doesn't make the default pickup sound when the pickup event is recieved
		makenoise = EV_ITEM_PICKUP_QUIET;
		G_AddEvent(activator, EV_GENERAL_SOUND, entity->noise_index);
	}
	// send the pickup event
	if (activator->client->pers.predictItemPickup) {
		G_AddPredictableEvent(activator, makenoise, entity->s.modelindex);
	} else {
		G_AddEvent(activator, makenoise, entity->s.modelindex);
	}
	// powerup pickups are global broadcasts
	if (entity->item->giType == IT_POWERUP || entity->item->giType == IT_TEAM) {
		// probably need to check for IT_KEY here too...(coop?)
		gentity_t *temp;

		temp = G_TempEntity(entity->s.pos.trBase, EV_GLOBAL_ITEM_PICKUP);
		temp->s.eventParm = entity->s.modelindex;
		temp->r.svFlags |= SVF_BROADCAST;
	}
	// fire item targets
	G_UseTargets(entity, activator);
	// wait of -1 will not respawn
	if (entity->wait == -1) {
		entity->flags |= FL_NODRAW;
		entity->r.svFlags |= SVF_NOCLIENT; // commented back in
		entity->s.eFlags |= EF_NODRAW;
		entity->r.contents = 0;
		entity->unlinkAfterEvent = qtrue;
		return;
	}
	// wait of -2 will respawn but not be available for pickup anymore (partial use things that leave a spent modle (ex. plate for turkey)
	if (respawn == RESPAWN_PARTIAL_DONE) {
		entity->s.density = (1 << 9); // (10 bits of data transmission for density)
		entity->active = qtrue; // re-activate
		trap_LinkEntity(entity);
		return;
	}

	if (respawn == RESPAWN_PARTIAL) { // multi-stage health
		entity->s.density--;

		if (entity->s.density) { // still not completely used up (this will change to == 0 and stage 1 will be a destroyable item (plate/etc.)
			entity->active = qtrue; // re-activate
			trap_LinkEntity(entity);
			return;
		}
	}
	// non zero wait overrides respawn time
	if (entity->wait) {
		respawn = entity->wait;
	}
	// random can be used to vary the respawn time
	if (entity->random) {
		respawn += crandom() * entity->random;

		if (respawn < 1) {
			respawn = 1;
		}
	}
	// dropped items will not respawn
	if (entity->flags & FL_DROPPED_ITEM) {
		entity->freeAfterEvent = qtrue;
	}
	// picked up items still stay around, they just don't draw anything. This allows respawnable items to be placed on movers
	entity->r.svFlags |= SVF_NOCLIENT;
	entity->flags |= FL_NODRAW;
	entity->r.contents = 0;
	// a negative respawn times means to never respawn this item (but don't delete it)
	// this is used by items that are respawned by third party events such as ctf flags
	if (respawn <= 0) {
		entity->nextthink = 0;
		entity->think = 0;
	} else {
		entity->nextthink = level.time + respawn * 1000;
		entity->think = RespawnItem;
	}

	trap_LinkEntity(entity);
}

/*
=======================================================================================================================================
Launch_Item

Spawns an item and tosses it forward.
=======================================================================================================================================
*/
gentity_t *Launch_Item(gitem_t *item, vec3_t origin, vec3_t velocity, int ownerNum) {
	gentity_t *dropped;
	trace_t tr;
	vec3_t vec, temp;
	int i;

	dropped = G_Spawn();
	dropped->s.eType = ET_ITEM;
	dropped->s.modelindex = item - bg_itemlist; // store item number in modelindex
	dropped->s.otherEntityNum2 = 1; // DHM - Nerve :: this is taking modelindex2's place for a dropped item
	dropped->classname = item->classname;
	dropped->item = item;

	VectorSet(dropped->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, 0); // so items sit on the ground
	VectorSet(dropped->r.maxs, ITEM_RADIUS, ITEM_RADIUS, 2 * ITEM_RADIUS); // so items sit on the ground

	dropped->r.contents = CONTENTS_TRIGGER|CONTENTS_ITEM;
	dropped->clipmask = CONTENTS_SOLID|CONTENTS_MISSILECLIP; // fix for items falling through grates
	dropped->touch = Touch_Item_Auto;

	trap_Trace(&tr, origin, dropped->r.mins, dropped->r.maxs, origin, ownerNum, MASK_SOLID);

	if (tr.startsolid) {
		VectorSubtract(g_entities[ownerNum].s.origin, origin, temp);
		VectorNormalize(temp);

		for (i = 16; i <= 48; i += 16) {
			VectorScale(temp, i, vec);
			VectorAdd(origin, vec, origin);

			trap_Trace(&tr, origin, dropped->r.mins, dropped->r.maxs, origin, ownerNum, MASK_SOLID);

			if (!tr.startsolid) {
				break;
			}
		}
	}

	G_SetOrigin(dropped, origin);
	dropped->s.pos.trType = TR_GRAVITY;
	dropped->s.pos.trTime = level.time;

	VectorCopy(velocity, dropped->s.pos.trDelta);

	dropped->s.eFlags |= EF_BOUNCE_HALF;

	if (item->giType == IT_TEAM) { // special case for CTF flags
		dropped->think = Team_DroppedFlagThink;
		dropped->nextthink = level.time + 30000;
	} else { // auto-remove after 30 seconds
		dropped->think = G_FreeEntity;
		dropped->nextthink = level.time + 30000;
	}

	dropped->flags = FL_DROPPED_ITEM;

	trap_LinkEntity(dropped);

	return dropped;
}

/*
=======================================================================================================================================
Drop_Item

Spawns an item and tosses it forward.
=======================================================================================================================================
*/
gentity_t *Drop_Item(gentity_t *ent, gitem_t *item, float angle, qboolean novelocity) {
	vec3_t velocity;
	vec3_t angles;

	VectorCopy(ent->s.apos.trBase, angles);

	angles[YAW] += angle;
	angles[PITCH] = 0; // always forward

	if (novelocity) {
		VectorClear(velocity);
	} else {
		AngleVectors(angles, velocity, NULL, NULL);
		VectorScale(velocity, 150, velocity);
		velocity[2] += 200 + crandom() * 50;
	}

	item->spawnflags |= FL_DROPPED;

	return Launch_Item(item, ent->s.pos.trBase, velocity, ent->s.number);
}

/*
=======================================================================================================================================
Use_Item

Respawn the item.
=======================================================================================================================================
*/
void Use_Item(gentity_t *ent, gentity_t *other, gentity_t *activator) {
	RespawnItem(ent);
}

/*
=======================================================================================================================================
FinishSpawningItem

Traces down to find where an item should rest, instead of letting them free fall from their spawn points.
=======================================================================================================================================
*/
void FinishSpawningItem(gentity_t *ent) {
	trace_t tr;
	vec3_t dest;
	vec3_t maxs;

	if (ent->spawnflags & FL_SUSPENDED) { // suspended
		VectorSet(ent->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
		VectorSet(ent->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
		VectorCopy(ent->r.maxs, maxs);
	} else {
		// had to modify this so that items would spawn in shelves
		VectorSet(ent->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, 0);
		VectorSet(ent->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
		VectorCopy(ent->r.maxs, maxs);
		maxs[2] /= 2;
	}

	ent->r.contents = CONTENTS_TRIGGER|CONTENTS_ITEM;
	ent->touch = Touch_Item_Auto;
	ent->s.eType = ET_ITEM;
	ent->s.modelindex = ent->item - bg_itemlist; // store item number in modelindex
	ent->s.otherEntityNum2 = 0; // takes modelindex2's place in signaling a dropped item
	// we don't use this (yet, anyway) so I'm taking it so you can specify a model for treasure items and clipboards
	//ent->s.modelindex2 = 0; // zero indicates this isn't a dropped item

	if (ent->model) {
		ent->s.modelindex2 = G_ModelIndex(ent->model);
	}
	// if clipboard, add the menu name string to the client's configstrings
	if (ent->item->giType == IT_CLIPBOARD) {
		if (!ent->message) {
			ent->s.density = G_FindConfigstringIndex("clip_test", CS_CLIPBOARDS, MAX_CLIPBOARD_CONFIGSTRINGS, qtrue);
		} else {
			ent->s.density = G_FindConfigstringIndex(ent->message, CS_CLIPBOARDS, MAX_CLIPBOARD_CONFIGSTRINGS, qtrue);
		}

		ent->touch = Touch_Item; // no auto-pickup, only activate
	} else if (ent->item->giType == IT_HOLDABLE) {
		if (ent->item->giTag >= HI_BOOK1 && ent->item->giTag <= HI_BOOK3) {
			G_FindConfigstringIndex(va("hbook%d", ent->item->giTag - HI_BOOK1), CS_CLIPBOARDS, MAX_CLIPBOARD_CONFIGSTRINGS, qtrue);
		}
//		ent->touch = Touch_Item; // no auto-pickup, only activate
	}
	// using an item causes it to respawn
	ent->use = Use_Item;
	// moved this up so it happens for suspended items too (and made it a function)
	G_SetAngle(ent, ent->s.angles);

	if (ent->spawnflags & FL_SUSPENDED) {
		// suspended
		G_SetOrigin(ent, ent->s.origin);
	} else {
		// drop to floor
		VectorSet(dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096);
		trap_Trace(&tr, ent->s.origin, ent->r.mins, maxs, dest, ent->s.number, MASK_SOLID);

		if (tr.startsolid) {
			vec3_t temp;

			VectorCopy(ent->s.origin, temp);
			temp[2] -= ITEM_RADIUS;

			VectorSet(dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096);
			trap_Trace(&tr, temp, ent->r.mins, maxs, dest, ent->s.number, MASK_SOLID);
		}
#if 0
		// drop to floor
		VectorSet(dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096);
		trap_Trace(&tr, ent->s.origin, ent->r.mins, maxs, dest, ent->s.number, MASK_SOLID);
#endif
		if (tr.startsolid) {
			G_Printf("FinishSpawningItem: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
			G_FreeEntity(ent);
			return;
		}
		// allow to ride movers
		ent->s.groundEntityNum = tr.entityNum;

		G_SetOrigin(ent, tr.endpos);
	}

	if (ent->spawnflags & FL_SPIN) { // spin
		ent->s.eFlags |= EF_SPINNING;
	}
	// team slaves and targeted items aren't present at start
	if ((ent->flags & FL_TEAMSLAVE) || ent->targetname) {
		ent->flags |= FL_NODRAW;
		//ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		return;
	}
	// health/ammo can potentially be multi-stage (multiple use)
	if (ent->item->giType == IT_HEALTH || ent->item->giType == IT_AMMO || ent->item->giType == IT_POWERUP) {
		int i;

		// having alternate models defined in bg_misc.c for a health or ammo item specify it as "multi-stage"
		// left-hand operand of comma expression has no effect
		// was: for(i = 0;i<4,ent->item->world_model[i];i++) {}

		i = 0;

		while (i < 4 && ent->item->world_model[i]) {
			i++;
		}

		ent->s.density = i - 1; // store number of stages in 'density' for client(most will have '1')
	}

	trap_LinkEntity(ent);
}

qboolean itemRegistered[MAX_ITEMS];
/*
=======================================================================================================================================
G_CheckTeamItems
=======================================================================================================================================
*/
void G_CheckTeamItems(void) {

}

/*
=======================================================================================================================================
ClearRegisteredItems
=======================================================================================================================================
*/
void ClearRegisteredItems(void) {
	memset(itemRegistered, 0, sizeof(itemRegistered));
	// players always start with the base weapon
	// Nope, not any more...
	// this will be determined by the level or starting position, or the savegame, but for now, re-register the MP40 automatically
	//RegisterItem(BG_FindItemForWeapon(WP_MP40));
	RegisterItem(BG_FindItem("Med Health")); // this is so med packs properly display
}

/*
=======================================================================================================================================
RegisterItem

The item will be added to the precache list.
=======================================================================================================================================
*/
void RegisterItem(gitem_t *item) {

	if (!item) {
		G_Error("RegisterItem: NULL");
	}

	itemRegistered[item - bg_itemlist] = qtrue;
}

/*
=======================================================================================================================================
SaveRegisteredItems

Write the needed items to a config string so the client will know which ones to precache.
=======================================================================================================================================
*/
void SaveRegisteredItems(void) {
	char string[MAX_ITEMS + 1];
	int i;
	int count;

	count = 0;

	for (i = 0; i < bg_numItems; i++) {
		if (itemRegistered[i]) {
			count++;
			string[i] = '1';
		} else {
			string[i] = '0';
		}
	}

	string[bg_numItems] = 0;

	trap_SetConfigstring(CS_ITEMS, string);
}

/*
=======================================================================================================================================
G_SpawnItem

Sets the clipping size and plants the object on the floor.
Items can't be immediately dropped to floor, because they might be on an entity that hasn't spawned yet.
=======================================================================================================================================
*/
void G_SpawnItem(gentity_t *ent, gitem_t *item) {
	char *noise;
	int page;

	G_SpawnFloat("random", "0", &ent->random);
	G_SpawnFloat("wait", "0", &ent->wait);

	RegisterItem(item);

	ent->item = item;
	// some movers spawn on the second frame, so delay item spawns until the third frame so they can ride trains
	ent->nextthink = level.time + FRAMETIME * 2;
	ent->think = FinishSpawningItem;

	if (G_SpawnString("noise", 0, &noise)) {
		ent->noise_index = G_SoundIndex(noise);
	}

	ent->physicsBounce = 0.50; // items are bouncy

	if (ent->model) {
		ent->s.modelindex2 = G_ModelIndex(ent->model);
	}

	if (item->giType == IT_CLIPBOARD) {
		if (G_SpawnInt("notebookpage", "1", &page)) {
			ent->key = page;
		}
	}

	if (item->giType == IT_POWERUP) {
		G_SoundIndex("sound/items/poweruprespawn.wav");
	}

	if (item->giType == IT_TREASURE) {
		level.numTreasure++;
		G_SendMissionStats();
	}
}

/*
=======================================================================================================================================
G_BounceItem
=======================================================================================================================================
*/
void G_BounceItem(gentity_t *ent, trace_t *trace) {
	vec3_t velocity;
	float dot;
	int hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + (level.time - level.previousTime) * trace->fraction;

	BG_EvaluateTrajectoryDelta(&ent->s.pos, hitTime, velocity);

	dot = DotProduct(velocity, trace->plane.normal);

	VectorMA(velocity, -2 * dot, trace->plane.normal, ent->s.pos.trDelta);
	// cut the velocity to keep from bouncing forever
	VectorScale(ent->s.pos.trDelta, ent->physicsBounce, ent->s.pos.trDelta);
	// check for stop
	if (trace->plane.normal[2] > 0 && ent->s.pos.trDelta[2] < 40) {
		trace->endpos[2] += 1.0; // make sure it is off ground
		SnapVector(trace->endpos);
		G_SetOrigin(ent, trace->endpos);

		ent->s.groundEntityNum = trace->entityNum;
		return;
	}

	VectorAdd(ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase);

	ent->s.pos.trTime = level.time;
}

/*
=======================================================================================================================================
G_RunItemProp
=======================================================================================================================================
*/
void G_RunItemProp(gentity_t *ent, vec3_t origin) {
	gentity_t *traceEnt;
	trace_t trace;
	gentity_t *owner;
	vec3_t start;
	vec3_t end;

	owner = &g_entities[ent->r.ownerNum];

	VectorCopy(ent->r.currentOrigin, start);
	start[2] += 1;

	VectorCopy(origin, end);
	end[2] += 1;

	trap_Trace(&trace, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, end, ent->r.ownerNum, MASK_SHOT);

	traceEnt = &g_entities[trace.entityNum];

	if (traceEnt && traceEnt->takedamage && traceEnt != ent) {
		ent->enemy = traceEnt;
	}

	if (owner->client && trace.startsolid && traceEnt != owner && traceEnt != ent /* && !traceEnt->active*/) {
		ent->takedamage = qfalse;
		ent->die(ent, ent, NULL, 10, 0);
		Prop_Break_Sound(ent);
		return;
	} else if (trace.surfaceFlags & SURF_NOIMPACT) {
		ent->takedamage = qfalse;

		Props_Chair_Skyboxtouch(ent);
		return;
	}
}

/*
=======================================================================================================================================
G_RunItem
=======================================================================================================================================
*/
void G_RunItem(gentity_t *ent) {
	vec3_t origin;
	trace_t tr;
	int contents;
	int mask;

	// if its groundentity has been set to none, it may have been pushed off an edge
	if (ent->s.groundEntityNum == ENTITYNUM_NONE) {
		if (ent->s.pos.trType != TR_GRAVITY) {
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trTime = level.time;
		}
	}

	if (ent->s.pos.trType == TR_STATIONARY || ent->s.pos.trType == TR_GRAVITY_PAUSED) {
		// check think function
		G_RunThink(ent);
		return;
	}
	// get current position
	BG_EvaluateTrajectory(&ent->s.pos, level.time, origin);
	// trace a line from the previous position to the current position
	if (ent->clipmask) {
		mask = ent->clipmask;
	} else {
		mask = MASK_SOLID|CONTENTS_MISSILECLIP;
	}

	trap_Trace(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->r.ownerNum, mask);

	if (ent->isProp && ent->takedamage) {
		G_RunItemProp(ent, origin);
	}

	VectorCopy(tr.endpos, ent->r.currentOrigin);

	if (tr.startsolid) {
		tr.fraction = 0;
	}

	trap_LinkEntity(ent); // FIXME: avoid this for stationary?
	// check think function
	G_RunThink(ent);

	if (tr.fraction == 1) {
		return;
	}
	// if it is in a nodrop volume, remove it
	contents = trap_PointContents(ent->r.currentOrigin, -1);

	if (contents & CONTENTS_NODROP) {
		if (ent->item && ent->item->giType == IT_TEAM) {
			Team_FreeEntity(ent);
		} else {
			G_FreeEntity(ent);
		}

		return;
	}

	G_BounceItem(ent, &tr);
}

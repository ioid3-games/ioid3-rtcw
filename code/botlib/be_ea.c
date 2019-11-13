/*
=======================================================================================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright(C)1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code(RTCW SP Source Code). 

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option)any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code. If not, see <http:// www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code. If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

=======================================================================================================================================
*/

/**************************************************************************************************************************************
 Elementary actions.
**************************************************************************************************************************************/

#include "../qcommon/q_shared.h"
#include "l_memory.h"
#include "l_script.h"
#include "l_precomp.h"
#include "l_struct.h"
#include "botlib.h"
#include "be_interface.h"
#include "be_ea.h"

#define MAX_USERMOVE 400
#define ACTION_JUMPEDLASTFRAME 128

bot_input_t *botinputs;

/*
=======================================================================================================================================
EA_Say
=======================================================================================================================================
*/
void EA_Say(int client, char *str) {
	botimport.BotClientCommand(client, va("say %s", str));
}

/*
=======================================================================================================================================
EA_SayTeam
=======================================================================================================================================
*/
void EA_SayTeam(int client, char *str) {
	botimport.BotClientCommand(client, va("say_team %s", str));
}

/*
=======================================================================================================================================
EA_UseItem
=======================================================================================================================================
*/
void EA_UseItem(int client, char *it) {
	botimport.BotClientCommand(client, va("use %s", it));
}

/*
=======================================================================================================================================
EA_DropItem
=======================================================================================================================================
*/
void EA_DropItem(int client, char *it) {
	botimport.BotClientCommand(client, va("drop %s", it));
}

/*
=======================================================================================================================================
EA_UseInv
=======================================================================================================================================
*/
void EA_UseInv(int client, char *inv) {
	botimport.BotClientCommand(client, va("invuse %s", inv));
}

/*
=======================================================================================================================================
EA_DropInv
=======================================================================================================================================
*/
void EA_DropInv(int client, char *inv) {
	botimport.BotClientCommand(client, va("invdrop %s", inv));
}

/*
=======================================================================================================================================
EA_Gesture
=======================================================================================================================================
*/
void EA_Gesture(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];

	bi->actionflags |= ACTION_GESTURE;
}

/*
=======================================================================================================================================
EA_Command
=======================================================================================================================================
*/
void EA_Command(int client, char *command) {
	botimport.BotClientCommand(client, command);
}

/*
=======================================================================================================================================
EA_SelectWeapon
=======================================================================================================================================
*/
void EA_SelectWeapon(int client, int weapon) {
	bot_input_t *bi;

	bi = &botinputs[client];
	bi->weapon = weapon;
}

/*
=======================================================================================================================================
EA_Attack
=======================================================================================================================================
*/
void EA_Attack(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];
	bi->actionflags |= ACTION_ATTACK;
}

/*
=======================================================================================================================================
EA_Reload
=======================================================================================================================================
*/
void EA_Reload(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];
	bi->actionflags |= ACTION_RELOAD;
}

/*
=======================================================================================================================================
EA_Talk
=======================================================================================================================================
*/
void EA_Talk(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];
	bi->actionflags |= ACTION_TALK;
}

/*
=======================================================================================================================================
EA_Use
=======================================================================================================================================
*/
void EA_Use(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];
	bi->actionflags |= ACTION_USE;
}

/*
=======================================================================================================================================
EA_Respawn
=======================================================================================================================================
*/
void EA_Respawn(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];
	bi->actionflags |= ACTION_RESPAWN;
}

/*
=======================================================================================================================================
EA_Jump
=======================================================================================================================================
*/
void EA_Jump(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];

	if (bi->actionflags & ACTION_JUMPEDLASTFRAME) {
		bi->actionflags &= ~ACTION_JUMP;
	} else {
		bi->actionflags |= ACTION_JUMP;
	}
}

/*
=======================================================================================================================================
EA_DelayedJump
=======================================================================================================================================
*/
void EA_DelayedJump(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];

	if (bi->actionflags & ACTION_JUMPEDLASTFRAME) {
		bi->actionflags &= ~ACTION_DELAYEDJUMP;
	} else {
		bi->actionflags |= ACTION_DELAYEDJUMP;
	}
}

/*
=======================================================================================================================================
EA_Crouch
=======================================================================================================================================
*/
void EA_Crouch(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];
	bi->actionflags |= ACTION_CROUCH;
}

/*
=======================================================================================================================================
EA_Walk
=======================================================================================================================================
*/
void EA_Walk(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];
	bi->actionflags |= ACTION_WALK;
}

/*
=======================================================================================================================================
EA_MoveUp
=======================================================================================================================================
*/
void EA_MoveUp(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];
	bi->actionflags |= ACTION_MOVEUP;
}

/*
=======================================================================================================================================
EA_MoveDown
=======================================================================================================================================
*/
void EA_MoveDown(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];
	bi->actionflags |= ACTION_MOVEDOWN;
}

/*
=======================================================================================================================================
EA_MoveForward
=======================================================================================================================================
*/
void EA_MoveForward(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];
	bi->actionflags |= ACTION_MOVEFORWARD;
}

/*
=======================================================================================================================================
EA_MoveBack
=======================================================================================================================================
*/
void EA_MoveBack(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];
	bi->actionflags |= ACTION_MOVEBACK;
}

/*
=======================================================================================================================================
EA_MoveLeft
=======================================================================================================================================
*/
void EA_MoveLeft(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];
	bi->actionflags |= ACTION_MOVELEFT;
}

/*
=======================================================================================================================================
EA_MoveRight
=======================================================================================================================================
*/
void EA_MoveRight(int client) {
	bot_input_t *bi;

	bi = &botinputs[client];
	bi->actionflags |= ACTION_MOVERIGHT;
}

/*
=======================================================================================================================================
EA_Move
=======================================================================================================================================
*/
void EA_Move(int client, vec3_t dir, float speed) {
	bot_input_t *bi;

	bi = &botinputs[client];

	VectorCopy(dir, bi->dir);
	// cap speed
	if (speed > MAX_USERMOVE) {
		speed = MAX_USERMOVE;
	} else if (speed < -MAX_USERMOVE) {
		speed = -MAX_USERMOVE;
	}

	bi->speed = speed;
}

/*
=======================================================================================================================================
EA_View
=======================================================================================================================================
*/
void EA_View(int client, vec3_t viewangles) {
	bot_input_t *bi;

	bi = &botinputs[client];

	VectorCopy(viewangles, bi->viewangles);
}

/*
=======================================================================================================================================
EA_EndRegular
=======================================================================================================================================
*/
void EA_EndRegular(int client, float thinktime) {
}

/*
=======================================================================================================================================
EA_GetInput
=======================================================================================================================================
*/
void EA_GetInput(int client, float thinktime, bot_input_t *input) {
	bot_input_t *bi;

	bi = &botinputs[client];
	bi->thinktime = thinktime;

	memcpy(input, bi, sizeof(bot_input_t));
}

/*
=======================================================================================================================================
EA_ResetInput
=======================================================================================================================================
*/
void EA_ResetInput(int client, bot_input_t *init) {
	bot_input_t *bi;
	int jumped = qfalse;

	bi = &botinputs[client];
	bi->thinktime = 0;

	VectorClear(bi->dir);

	bi->speed = 0;
	jumped = bi->actionflags & ACTION_JUMP;
	bi->actionflags = 0;

	if (jumped) {
		bi->actionflags |= ACTION_JUMPEDLASTFRAME;
	}

	if (init) {
		memcpy(bi, init, sizeof(bot_input_t));
	}
}

/*
=======================================================================================================================================
EA_Setup
=======================================================================================================================================
*/
int EA_Setup(void) {
	// initialize the bot inputs
	botinputs = (bot_input_t *)GetClearedHunkMemory(botlibglobals.maxclients * sizeof(bot_input_t));
	return BLERR_NOERROR;
}

/*
=======================================================================================================================================
EA_Shutdown
=======================================================================================================================================
*/
void EA_Shutdown(void) {
	FreeMemory(botinputs);
	botinputs = NULL;
}

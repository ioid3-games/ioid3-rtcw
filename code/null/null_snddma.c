/*
=======================================================================================================================================

Return to Castle Wolfenstein multiplayer GPL Source Code
Copyright(C)1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein multiplayer GPL Source Code(RTCW MP Source Code). 

RTCW MP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option)any later version.

RTCW MP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW MP Source Code. If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW MP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW MP Source Code. If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

=======================================================================================================================================
*/

// all other sound mixing is portable

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

/*
=======================================================================================================================================
SNDDMA_Init
=======================================================================================================================================
*/
qboolean SNDDMA_Init(void) {
	return qfalse;
}

/*
=======================================================================================================================================
SNDDMA_GetDMAPos
=======================================================================================================================================
*/
int SNDDMA_GetDMAPos(void) {
	return 0;
}

/*
=======================================================================================================================================
SNDDMA_Shutdown
=======================================================================================================================================
*/
void SNDDMA_Shutdown(void) {

}

/*
=======================================================================================================================================
SNDDMA_BeginPainting
=======================================================================================================================================
*/
void SNDDMA_BeginPainting(void) {

}

/*
=======================================================================================================================================
SNDDMA_Submit
=======================================================================================================================================
*/
void SNDDMA_Submit(void) {

}
#ifdef USE_VOIP
/*
=======================================================================================================================================
SNDDMA_StartCapture
=======================================================================================================================================
*/
void SNDDMA_StartCapture(void) {

}

/*
=======================================================================================================================================
SNDDMA_AvailableCaptureSamples
=======================================================================================================================================
*/
int SNDDMA_AvailableCaptureSamples(void) {
	return 0;
}
/*
=======================================================================================================================================
SNDDMA_Capture
=======================================================================================================================================
*/
void SNDDMA_Capture(int samples, byte *data) {

}

/*
=======================================================================================================================================
SNDDMA_StopCapture
=======================================================================================================================================
*/
void SNDDMA_StopCapture(void) {

}

/*
=======================================================================================================================================
SNDDMA_MasterGain
=======================================================================================================================================
*/
void SNDDMA_MasterGain(float val) {

}
#endif
/*
=======================================================================================================================================
S_RegisterSound
=======================================================================================================================================
*/
sfxHandle_t S_RegisterSound(const char *name, qboolean compressed) {
	return 0;
}

/*
=======================================================================================================================================
S_StartLocalSound
=======================================================================================================================================
*/
void S_StartLocalSound(sfxHandle_t sfxHandle, int channelNum) {

}

/*
=======================================================================================================================================
S_ClearSoundBuffer
=======================================================================================================================================
*/
void S_ClearSoundBuffer(void) {

}

/*
=======================================================================================================================================
SNDDMA_Activate

Added for win32 dedicated.
=======================================================================================================================================
*/
void SNDDMA_Activate(void) {

}

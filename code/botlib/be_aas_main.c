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
 AAS
**************************************************************************************************************************************/

#include "../qcommon/q_shared.h"
#include "l_memory.h"
#include "l_libvar.h"
#include "l_utils.h"
#include "l_script.h"
#include "l_precomp.h"
#include "l_struct.h"
#include "l_log.h"
#include "aasfile.h"
#include "botlib.h"
#include "be_aas.h"
#include "be_aas_funcs.h"
#include "be_interface.h"
#include "be_aas_def.h"

aas_t aasworlds[MAX_AAS_WORLDS];
aas_t *aasworld;

/*
=======================================================================================================================================
AAS_Error
=======================================================================================================================================
*/
void QDECL AAS_Error(char *fmt, ...) {
	char str[1024];
	va_list arglist;

	va_start(arglist, fmt);
	Q_vsnprintf(str, sizeof(str), fmt, arglist);
	va_end(arglist);

	botimport.Print(PRT_FATAL, "%s", str);
}

/*
=======================================================================================================================================
AAS_SetCurrentWorld

Multiple AAS worlds.
=======================================================================================================================================
*/
void AAS_SetCurrentWorld(int index) {

	if (index >= MAX_AAS_WORLDS || index < 0) {
		AAS_Error("AAS_SetCurrentWorld: index out of range\n");
		return;
	}
	// set the current world pointer
	aasworld = &aasworlds[index];
}

/*
=======================================================================================================================================
AAS_StringFromIndex
=======================================================================================================================================
*/
char *AAS_StringFromIndex(char *indexname, char *stringindex[], int numindexes, int index) {

	if (!(*aasworld).indexessetup) {
		botimport.Print(PRT_ERROR, "%s: index %d not setup\n", indexname, index);
		return "";
	}

	if (index < 0 || index >= numindexes) {
		botimport.Print(PRT_ERROR, "%s: index %d out of range\n", indexname, index);
		return "";
	}

	if (!stringindex[index]) {
		if (index) {
			botimport.Print(PRT_ERROR, "%s: reference to unused index %d\n", indexname, index);
		}

		return "";
	}

	return stringindex[index];
}

/*
=======================================================================================================================================
AAS_IndexFromString
=======================================================================================================================================
*/
int AAS_IndexFromString(char *indexname, char *stringindex[], int numindexes, char *string) {
	int i;

	if (!(*aasworld).indexessetup) {
		botimport.Print(PRT_ERROR, "%s: index not setup \"%s\"\n", indexname, string);
		return 0;
	}

	for (i = 0; i < numindexes; i++) {
		if (!stringindex[i]) {
			continue;
		}

		if (!Q_stricmp(stringindex[i], string)) {
			return i;
		}
	}

	return 0;
}

/*
=======================================================================================================================================
AAS_ModelFromIndex
=======================================================================================================================================
*/
char *AAS_ModelFromIndex(int index) {
	return 0; // removed so the CS_ defines could be removed from be_aas_def.h
}

/*
=======================================================================================================================================
AAS_IndexFromModel
=======================================================================================================================================
*/
int AAS_IndexFromModel(char *modelname) {
	return 0; // removed so the CS_ defines could be removed from be_aas_def.h
}

/*
=======================================================================================================================================
AAS_UpdateStringIndexes
=======================================================================================================================================
*/
void AAS_UpdateStringIndexes(int numconfigstrings, char *configstrings[]) {
	int i;

	// set string pointers and copy the strings
	for (i = 0; i < numconfigstrings; i++) {
		if (configstrings[i]) {
			(*aasworld).configstrings[i] = (char *)GetMemory(strlen(configstrings[i]) + 1);
			strcpy((*aasworld).configstrings[i], configstrings[i]);
		}
	}

	(*aasworld).indexessetup = qtrue;
}

/*
=======================================================================================================================================
AAS_Loaded
=======================================================================================================================================
*/
int AAS_Loaded(void) {
	return (*aasworld).loaded;
}

/*
=======================================================================================================================================
AAS_Initialized
=======================================================================================================================================
*/
int AAS_Initialized(void) {
	return (*aasworld).initialized;
}

/*
=======================================================================================================================================
AAS_SetInitialized
=======================================================================================================================================
*/
void AAS_SetInitialized(void) {

	(*aasworld).initialized = qtrue;
	botimport.Print(PRT_MESSAGE, "AAS initialized.\n");
#ifdef DEBUG
	// create all the routing cache
	//AAS_CreateAllRoutingCache();
	//AAS_RoutingInfo();
#endif
	// build/load the route-table
	AAS_RT_BuildRouteTable();
}

/*
=======================================================================================================================================
AAS_ContinueInit
=======================================================================================================================================
*/
void AAS_ContinueInit(float time) {
	// if no AAS file loaded
	if (!(*aasworld).loaded) {
		return;
	}
	// if AAS is already initialized
	if ((*aasworld).initialized) {
		return;
	}
	// calculate reachability, if not finished return
	if (AAS_ContinueInitReachability(time)) {
		return;
	}
	// initialize clustering for the new map
	AAS_InitClustering();
	// if reachability has been calculated and an AAS file should be written or there is a forced data optimization
	if ((*aasworld).savefile || ((int)LibVarGetValue("forcewrite"))) {
		// optimize the AAS data
		if (!((int)LibVarValue("nooptimize", "1"))) {
			AAS_Optimize();
		}
		// save the AAS file
		if (AAS_WriteAASFile((*aasworld).filename)) {
			botimport.Print(PRT_MESSAGE, "%s written successfully\n", (*aasworld).filename);
		} else {
			botimport.Print(PRT_ERROR, "couldn't write %s\n", (*aasworld).filename);
		}
	}
	// initialize the routing
	AAS_InitRouting();
	// at this point AAS is initialized
	AAS_SetInitialized();
}

/*
=======================================================================================================================================
AAS_StartFrame

Called at the start of every frame.
=======================================================================================================================================
*/
int AAS_StartFrame(float time) {
	// do each of the aasworlds
	int i;

	for (i = 0; i < MAX_AAS_WORLDS; i++) {
		AAS_SetCurrentWorld(i);

		(*aasworld).time = time;
		// invalidate the entities
		AAS_InvalidateEntities();
		// initialize AAS
		AAS_ContinueInit(time);

		(*aasworld).frameroutingupdates = 0;
		/* disabled for speed
		if (LibVarGetValue("showcacheupdates")) {
			AAS_RoutingInfo();
			LibVarSet("showcacheupdates", "0");
		}

		if (LibVarGetValue("showmemoryusage")) {
			PrintUsedMemorySize();
			LibVarSet("showmemoryusage", "0");
		}

		if (LibVarGetValue("memorydump")) {
			PrintMemoryLabels();
			LibVarSet("memorydump", "0");
		}
		*/
	}

	(*aasworld).numframes++;
	return BLERR_NOERROR;
}

/*
=======================================================================================================================================
AAS_Time
=======================================================================================================================================
*/
float AAS_Time(void) {
	return (*aasworld).time;
}

/*
=======================================================================================================================================
AAS_LoadFiles

 basedir	= Quake2 console basedir
 gamedir	= Quake2 console gamedir
 mapname	= name of the map without extension(.bsp)
=======================================================================================================================================
*/
int AAS_LoadFiles(const char *mapname) {
	int errnum;
	char aasfile[MAX_QPATH];

	Q_strncpyz((*aasworld).mapname, mapname, sizeof((*aasworld).mapname));
	// NOTE: first reset the entity links into the AAS areas and BSP leaves
	// the AAS link heap and BSP link heap are reset after respectively the AAS file and BSP file are loaded
	AAS_ResetEntityLinks();
	// load bsp info
	AAS_LoadBSPFile();
	// load the aas file
	Com_sprintf(aasfile, sizeof(aasfile), "maps/%s.aas", mapname);

	errnum = AAS_LoadAASFile(aasfile);

	if (errnum != BLERR_NOERROR) {
		return errnum;
	}

	botimport.Print(PRT_MESSAGE, "loaded %s\n", aasfile);

	Q_strncpyz((*aasworld).filename, aasfile, sizeof((*aasworld).filename));
	return BLERR_NOERROR;
}

/*
=======================================================================================================================================
AAS_LoadMap

Called every time a map changes. Modified this for multiple AAS files.
=======================================================================================================================================
*/
int AAS_LoadMap(const char *mapname) {
#define MAPNAME_LEN 256
	int errnum;
	int i;
	char this_mapname[MAPNAME_LEN], intstr[4];
	qboolean loaded = qfalse;
	int missingErrNum = 0;

	for (i = 0; i < MAX_AAS_WORLDS; i++) {
		AAS_SetCurrentWorld(i);

		Q_strncpyz(this_mapname, mapname, sizeof(this_mapname));
		Q_strcat(this_mapname, sizeof(this_mapname) - strlen(this_mapname) - 1, "_b");
		Com_sprintf(intstr, sizeof(intstr), "%i", i);
		Q_strcat(this_mapname, sizeof(this_mapname) - strlen(this_mapname) - 1, intstr);
		// if no mapname is provided then the string indexes are updated
		if (!mapname) {
			return 0;
		}

		(*aasworld).initialized = qfalse;
		// NOTE: free the routing caches before loading a new map because to free the caches the old number of areas, number of clusters
		// and number of areas in a clusters must be available
		AAS_FreeRoutingCaches();
		// load the map
		errnum = AAS_LoadFiles(this_mapname);

		if (errnum != BLERR_NOERROR) {
			(*aasworld).loaded = qfalse;
			// we are allowed to skip one of the files, but not both
			//return errnum;
			missingErrNum = errnum;
			continue;
		}

		loaded = qtrue;

		AAS_InitSettings();
		// initialize the AAS link heap for the new map
		AAS_InitAASLinkHeap();
		// initialize the AAS linked entities for the new map
		AAS_InitAASLinkedEntities();
		// initialize reachability for the new map
		AAS_InitReachability();
		// initialize the alternative routing
		AAS_InitAlternativeRouting();
	}

	if (!loaded) {
		return missingErrNum;
	}
	// everything went ok
	return 0;
}

/*
=======================================================================================================================================
AAS_Setup

Called when the library is first loaded.
=======================================================================================================================================
*/
int AAS_Setup(void) {
	// Ridah, just use the default world for entities
	AAS_SetCurrentWorld(0);

	(*aasworlds).maxclients = (int)LibVarValue("maxclients", "128");
	(*aasworlds).maxentities = (int)LibVarValue("maxentities", "2048");
	// allocate memory for the entities
	if ((*aasworld).entities) {
		FreeMemory((*aasworld).entities);
	}

	(*aasworld).entities = (aas_entity_t *)GetClearedHunkMemory((*aasworld).maxentities * sizeof(aas_entity_t));
	// invalidate all the entities
	AAS_InvalidateEntities();
	// force some recalculations
	//LibVarSet("forceclustering", "1"); // force clustering calculation
	//LibVarSet("forcereachability", "1"); // force reachability calculation
	(*aasworld).numframes = 0;
	return BLERR_NOERROR;
}

/*
=======================================================================================================================================
AAS_Shutdown
=======================================================================================================================================
*/
void AAS_Shutdown(void) {
	// do each of the worlds
	int i;

	for (i = 0; i < MAX_AAS_WORLDS; i++) {
		AAS_SetCurrentWorld(i);
		// Ridah, kill the route-table data
		AAS_RT_ShutdownRouteTable();
		AAS_ShutdownAlternativeRouting();
		AAS_DumpBSPData();
		// free routing caches
		AAS_FreeRoutingCaches();
		// free aas link heap
		AAS_FreeAASLinkHeap();
		// free aas linked entities
		AAS_FreeAASLinkedEntities();
		// free the aas data
		AAS_DumpAASData();

		if (i == 0) {
			// free the entities
			if ((*aasworld).entities) {
				FreeMemory((*aasworld).entities);
			}
		}
		// clear the(*aasworld)structure
		memset(&(*aasworld), 0, sizeof(aas_t));
		// aas has not been initialized
		(*aasworld).initialized = qfalse;
	}
	// NOTE: as soon as a new .bsp file is loaded the .bsp file memory is freed and reallocated, so there's no need to free that memory here
	// print shutdown
	botimport.Print(PRT_MESSAGE, "AAS shutdown.\n");
}

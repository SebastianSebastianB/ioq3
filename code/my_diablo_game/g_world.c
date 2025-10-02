#include "g_local.h"

#define JSON_IMPLEMENTATION
#include "../qcommon/json.h"
#undef JSON_IMPLEMENTATION

#define MAX_WORLD_FILE_SIZE 65536
#define WORLD_PLAYER_START_KEY "player_start"

typedef struct worldDefinition_s {
	qboolean	active;
	char		fileName[MAX_QPATH];
	vec3_t		playerStartOrigin;
	qboolean	playerStartOriginSet;
	vec3_t		playerStartAngles;
	qboolean	playerStartAnglesSet;
} worldDefinition_t;

static worldDefinition_t g_world;
static qboolean g_warnedMissingPlayerStart;

static qboolean G_ParseWorldVec3( const char *json, const char *jsonEnd, vec3_t out ) {
	const char *components[3] = { NULL, NULL, NULL };
	int count;
	int jsonType;

	G_Printf( "DEBUG: G_ParseWorldVec3 called, json=%p, jsonEnd=%p\n", json, jsonEnd );

	if ( !json ) {
		G_Printf( "DEBUG: G_ParseWorldVec3 - json is NULL!\n" );
		return qfalse;
	}

	jsonType = JSON_ValueGetType( json, jsonEnd );
	G_Printf( "DEBUG: JSON_ValueGetType=%d (JSONTYPE_ARRAY=%d)\n", jsonType, JSONTYPE_ARRAY );

	if ( jsonType != JSONTYPE_ARRAY ) {
		G_Printf( "DEBUG: G_ParseWorldVec3 - Not an array type!\n" );
		return qfalse;
	}

	count = JSON_ArrayGetIndex( json, jsonEnd, components, 3 );
	G_Printf( "DEBUG: JSON_ArrayGetIndex returned count=%d\n", count );

	if ( count < 3 ) {
		G_Printf( "DEBUG: G_ParseWorldVec3 - Count < 3!\n" );
		return qfalse;
	}

	for ( int i = 0; i < 3; ++i ) {
		if ( !components[i] ) {
			G_Printf( "DEBUG: G_ParseWorldVec3 - component[%d] is NULL!\n", i );
			return qfalse;
		}

		out[i] = JSON_ValueGetFloat( components[i], jsonEnd );
		G_Printf( "DEBUG: G_ParseWorldVec3 - out[%d] = %f\n", i, out[i] );
	}

	G_Printf( "DEBUG: G_ParseWorldVec3 - SUCCESS! (%f, %f, %f)\n", out[0], out[1], out[2] );
	return qtrue;
}

static void G_ParseWorldPlayerStart( const char *objectsJson, const char *jsonEnd ) {
	const char *playerStartJson;
	const char *originJson;
	const char *anglesJson;

	G_Printf( "DEBUG: G_ParseWorldPlayerStart called\n" );

	if ( !objectsJson ) {
		G_Printf( "DEBUG: objectsJson is NULL!\n" );
		return;
	}

	if ( JSON_ValueGetType( objectsJson, jsonEnd ) != JSONTYPE_OBJECT ) {
		G_Printf( "DEBUG: objectsJson is not an OBJECT!\n" );
		return;
	}

	playerStartJson = JSON_ObjectGetNamedValue( objectsJson, jsonEnd, WORLD_PLAYER_START_KEY );
	G_Printf( "DEBUG: playerStartJson=%p (after ObjectGetNamedValue for '%s')\n", playerStartJson, WORLD_PLAYER_START_KEY );

	if ( !playerStartJson ) {
		G_Printf( "DEBUG: playerStartJson is NULL!\n" );
		return;
	}

	if ( JSON_ValueGetType( playerStartJson, jsonEnd ) != JSONTYPE_OBJECT ) {
		G_Printf( "DEBUG: playerStartJson is not an OBJECT!\n" );
		return;
	}

	originJson = JSON_ObjectGetNamedValue( playerStartJson, jsonEnd, "origin" );
	G_Printf( "DEBUG: originJson=%p (after ObjectGetNamedValue for 'origin')\n", originJson );

	anglesJson = JSON_ObjectGetNamedValue( playerStartJson, jsonEnd, "angles" );
	G_Printf( "DEBUG: anglesJson=%p (after ObjectGetNamedValue for 'angles')\n", anglesJson );

	g_world.playerStartOriginSet = G_ParseWorldVec3( originJson, jsonEnd, g_world.playerStartOrigin );
	G_Printf( "DEBUG: After origin parsing - playerStartOriginSet=%d\n", g_world.playerStartOriginSet );

	g_world.playerStartAnglesSet = G_ParseWorldVec3( anglesJson, jsonEnd, g_world.playerStartAngles );
	G_Printf( "DEBUG: After angles parsing - playerStartAnglesSet=%d\n", g_world.playerStartAnglesSet );
}

void G_ClearWorldDefinition( void ) {
	Com_Memset( &g_world, 0, sizeof( g_world ) );
	g_warnedMissingPlayerStart = qfalse;
}

qboolean G_LoadWorldDefinition( const char *mapName ) {
	fileHandle_t handle;
	int length;
	static char buffer[MAX_WORLD_FILE_SIZE];
	const char *jsonEnd;
	char resolvedPath[MAX_QPATH];
	const char *pathToUse;

	G_Printf( "DEBUG: G_LoadWorldDefinition called with mapName='%s'\n", mapName ? mapName : "(null)" );

	G_ClearWorldDefinition();

	if ( !mapName || !COM_CompareExtension( mapName, ".world" ) ) {
		G_Printf( "DEBUG: G_LoadWorldDefinition - not a .world file\n" );
		return qfalse;
	}

	if ( strchr( mapName, '/' ) ) {
		pathToUse = mapName;
	} else {
		Com_sprintf( resolvedPath, sizeof( resolvedPath ), "maps/%s", mapName );
		pathToUse = resolvedPath;
	}

	G_Printf( "DEBUG: Trying to open file '%s'\n", pathToUse );
	length = trap_FS_FOpenFile( pathToUse, &handle, FS_READ );
	G_Printf( "DEBUG: trap_FS_FOpenFile returned length=%d, handle=%p\n", length, handle );
	if ( length <= 0 || !handle ) {
		G_Printf( "DEBUG: Failed to open world file '%s'\n", pathToUse );
		G_Error( "Failed to open world file '%s'", pathToUse );
	}

	if ( length >= MAX_WORLD_FILE_SIZE ) {
		trap_FS_FCloseFile( handle );
		G_Error( "World file '%s' exceeds maximum supported size", pathToUse );
	}

	trap_FS_Read( buffer, length, handle );
	trap_FS_FCloseFile( handle );
	buffer[length] = '\0';

	// Print first 500 characters of JSON for debugging
	G_Printf( "DEBUG: JSON file content (first 500 chars):\n" );
	if ( length > 500 ) {
		char tempBuf[501];
		Q_strncpyz( tempBuf, buffer, sizeof(tempBuf) );
		G_Printf( "%s...\n", tempBuf );
	} else {
		G_Printf( "%s\n", buffer );
	}

	jsonEnd = buffer + length;

	if ( JSON_ValueGetType( buffer, jsonEnd ) != JSONTYPE_OBJECT ) {
		G_Error( "World file '%s' must begin with a JSON object", pathToUse );
	}

	G_Printf( "DEBUG: About to call G_ParseWorldPlayerStart\n" );
	G_ParseWorldPlayerStart( JSON_ObjectGetNamedValue( buffer, jsonEnd, "objects" ), jsonEnd );
	G_Printf( "DEBUG: After G_ParseWorldPlayerStart - playerStartOriginSet=%d\n", g_world.playerStartOriginSet );

	Q_strncpyz( g_world.fileName, pathToUse, sizeof( g_world.fileName ) );
	g_world.active = qtrue;
	return qtrue;
}

qboolean G_WorldPlayerStart( vec3_t origin, vec3_t angles ) {
	if ( !g_world.active || !g_world.playerStartOriginSet ) {
		if ( g_world.active && !g_warnedMissingPlayerStart ) {
			G_Printf( S_COLOR_YELLOW "WARNING: world definition '%s' is missing player_start origin\n",
				g_world.fileName[0] ? g_world.fileName : "<unknown>" );
			g_warnedMissingPlayerStart = qtrue;
		}
		return qfalse;
	}

	VectorCopy( g_world.playerStartOrigin, origin );
	if ( g_world.playerStartAnglesSet ) {
		VectorCopy( g_world.playerStartAngles, angles );
	} else {
		VectorClear( angles );
	}
	return qtrue;
}

void G_InitWorldForDefinition( void ) {
	gentity_t *worldEnt = &g_entities[ENTITYNUM_WORLD];
	gentity_t *noneEnt = &g_entities[ENTITYNUM_NONE];

	trap_SetConfigstring( CS_GAME_VERSION, GAME_VERSION );
	trap_SetConfigstring( CS_LEVEL_START_TIME, va( "%i", level.startTime ) );
	trap_SetConfigstring( CS_MUSIC, "" );
	trap_SetConfigstring( CS_MESSAGE, "" );
	trap_SetConfigstring( CS_MOTD, g_motd.string );

	trap_Cvar_Set( "g_gravity", "800" );
	trap_Cvar_Set( "g_enableDust", "0" );
	trap_Cvar_Set( "g_enableBreath", "0" );

	worldEnt->s.number = ENTITYNUM_WORLD;
	worldEnt->r.ownerNum = ENTITYNUM_NONE;
	worldEnt->classname = "worldspawn";

	noneEnt->s.number = ENTITYNUM_NONE;
	noneEnt->r.ownerNum = ENTITYNUM_NONE;
	noneEnt->classname = "nothing";

	trap_SetConfigstring( CS_WARMUP, "" );
	if ( g_restarted.integer ) {
		trap_Cvar_Set( "g_restarted", "0" );
		level.warmupTime = 0;
	} else if ( g_doWarmup.integer ) {
		level.warmupTime = -1;
		trap_SetConfigstring( CS_WARMUP, va( "%i", level.warmupTime ) );
		G_LogPrintf( "Warmup:\n" );
	}
}
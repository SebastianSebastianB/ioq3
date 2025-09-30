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

	if ( !json || JSON_ValueGetType( json, jsonEnd ) != JSONTYPE_ARRAY ) {
		return qfalse;
	}

	count = JSON_ArrayGetIndex( json, jsonEnd, components, 3 );
	if ( count < 3 ) {
		return qfalse;
	}

	for ( int i = 0; i < 3; ++i ) {
		if ( !components[i] ) {
			return qfalse;
		}

		out[i] = JSON_ValueGetFloat( components[i], jsonEnd );
	}

	return qtrue;
}

static void G_ParseWorldPlayerStart( const char *objectsJson, const char *jsonEnd ) {
	const char *playerStartJson;

	if ( !objectsJson || JSON_ValueGetType( objectsJson, jsonEnd ) != JSONTYPE_OBJECT ) {
		return;
	}

	playerStartJson = JSON_ObjectGetNamedValue( objectsJson, jsonEnd, WORLD_PLAYER_START_KEY );
	if ( !playerStartJson || JSON_ValueGetType( playerStartJson, jsonEnd ) != JSONTYPE_OBJECT ) {
		return;
	}

	g_world.playerStartOriginSet = G_ParseWorldVec3( JSON_ObjectGetNamedValue( playerStartJson, jsonEnd, "origin" ), jsonEnd, g_world.playerStartOrigin );
	g_world.playerStartAnglesSet = G_ParseWorldVec3( JSON_ObjectGetNamedValue( playerStartJson, jsonEnd, "angles" ), jsonEnd, g_world.playerStartAngles );
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

	jsonEnd = buffer + length;

	if ( JSON_ValueGetType( buffer, jsonEnd ) != JSONTYPE_OBJECT ) {
		G_Error( "World file '%s' must begin with a JSON object", pathToUse );
	}

	G_ParseWorldPlayerStart( JSON_ObjectGetNamedValue( buffer, jsonEnd, "objects" ), jsonEnd );

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
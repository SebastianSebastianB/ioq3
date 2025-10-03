#include "g_local.h"

#define JSON_IMPLEMENTATION
#include "../qcommon/json.h"
#undef JSON_IMPLEMENTATION

#define MAX_WORLD_FILE_SIZE 65536
#define WORLD_PLAYER_START_KEY "player_start"
#define WORLD_SKYBOX_KEY "skybox"

typedef struct worldDefinition_s {
	qboolean	active;
	char		fileName[MAX_QPATH];
	char		skyboxShader[MAX_QPATH];
	vec3_t		playerStartOrigin;
	qboolean	playerStartOriginSet;
	vec3_t		playerStartAngles;
	qboolean	playerStartAnglesSet;
	
	// Terrain data - MUST match client cg_worldInfo_t.terrain!
	struct {
		qboolean	enabled;
		char		heightmap[MAX_QPATH];
		float		scaleHorizontal;
		float		scaleVertical;
	} terrain;
} worldDefinition_t;

static worldDefinition_t g_world;
static qboolean g_warnedMissingPlayerStart;

static qboolean G_ParseWorldVec3( const char *json, const char *jsonEnd, vec3_t out ) {
	const char *components[3] = { NULL, NULL, NULL };
	int count;
	int jsonType;

	

	if ( !json ) {
		
		return qfalse;
	}

	jsonType = JSON_ValueGetType( json, jsonEnd );
	

	if ( jsonType != JSONTYPE_ARRAY ) {
		
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

static void G_ParseWorldTerrain( const char *terrainJson, const char *jsonEnd ) {
	const char *value;

	if ( !terrainJson || JSON_ValueGetType( terrainJson, jsonEnd ) != JSONTYPE_OBJECT ) {
		return;
	}

	g_world.terrain.enabled = qtrue;

	// Parse heightmap path
	value = JSON_ObjectGetNamedValue( terrainJson, jsonEnd, "heightmap" );
	if ( value && JSON_ValueGetString( value, jsonEnd, g_world.terrain.heightmap, sizeof(g_world.terrain.heightmap) ) ) {
		G_Printf( "^2[G_ParseWorldTerrain] heightmap='%s'\n", g_world.terrain.heightmap );
	}

	// Parse scale object
	value = JSON_ObjectGetNamedValue( terrainJson, jsonEnd, "scale" );
	if ( value && JSON_ValueGetType( value, jsonEnd ) == JSONTYPE_OBJECT ) {
		const char *scaleValue;

		scaleValue = JSON_ObjectGetNamedValue( value, jsonEnd, "horizontal" );
		if ( scaleValue ) {
			g_world.terrain.scaleHorizontal = JSON_ValueGetFloat( scaleValue, jsonEnd );
			G_Printf( "^2[G_ParseWorldTerrain] scaleHorizontal=%.2f\n", g_world.terrain.scaleHorizontal );
		}

		scaleValue = JSON_ObjectGetNamedValue( value, jsonEnd, "vertical" );
		if ( scaleValue ) {
			g_world.terrain.scaleVertical = JSON_ValueGetFloat( scaleValue, jsonEnd );
			G_Printf( "^2[G_ParseWorldTerrain] scaleVertical=%.2f\n", g_world.terrain.scaleVertical );
		}
	}
}

static void G_ParseWorldPlayerStart( const char *objectsJson, const char *jsonEnd ) {
	const char *playerStartJson;
	const char *originJson;
	const char *anglesJson;

	

	if ( !objectsJson ) {
		
		return;
	}

	if ( JSON_ValueGetType( objectsJson, jsonEnd ) != JSONTYPE_OBJECT ) {
		
		return;
	}

	playerStartJson = JSON_ObjectGetNamedValue( objectsJson, jsonEnd, WORLD_PLAYER_START_KEY );
	

	if ( !playerStartJson ) {
		
		return;
	}

	if ( JSON_ValueGetType( playerStartJson, jsonEnd ) != JSONTYPE_OBJECT ) {
		
		return;
	}

	originJson = JSON_ObjectGetNamedValue( playerStartJson, jsonEnd, "origin" );
	

	anglesJson = JSON_ObjectGetNamedValue( playerStartJson, jsonEnd, "angles" );
	

	g_world.playerStartOriginSet = G_ParseWorldVec3( originJson, jsonEnd, g_world.playerStartOrigin );
	

	g_world.playerStartAnglesSet = G_ParseWorldVec3( anglesJson, jsonEnd, g_world.playerStartAngles );
	
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

	

	G_ClearWorldDefinition();

	if ( !mapName || !COM_CompareExtension( mapName, ".world" ) ) {
		
		return qfalse;
	}

	if ( strchr( mapName, '/' ) ) {
		pathToUse = mapName;
	} else {
		Com_sprintf( resolvedPath, sizeof( resolvedPath ), "maps/%s", mapName );
		pathToUse = resolvedPath;
	}

	
	length = trap_FS_FOpenFile( pathToUse, &handle, FS_READ );
	
	if ( length <= 0 || !handle ) {
		
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

	// Parse skybox shader name
	{
		const char *skyboxJson = JSON_ObjectGetNamedValue( buffer, jsonEnd, WORLD_SKYBOX_KEY );
		if ( skyboxJson && JSON_ValueGetType( skyboxJson, jsonEnd ) == JSONTYPE_STRING ) {
			char skyboxValue[MAX_QPATH];
			unsigned int len = JSON_ValueGetString( skyboxJson, jsonEnd, skyboxValue, sizeof(skyboxValue) );
			if ( len > 0 ) {
				Q_strncpyz( g_world.skyboxShader, skyboxValue, sizeof( g_world.skyboxShader ) );
				
			}
		}
	}

	// Parse terrain data - CRITICAL for server/client sync!
	G_ParseWorldTerrain( JSON_ObjectGetNamedValue( buffer, jsonEnd, "terrain" ), jsonEnd );
	
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

const char *G_WorldGetSkybox( void ) {
	if ( !g_world.active ) {
		return NULL;
	}
	return g_world.skyboxShader[0] ? g_world.skyboxShader : NULL;
}

qboolean G_WorldGetTerrainEnabled( void ) {
	return g_world.active && g_world.terrain.enabled;
}

float G_WorldGetTerrainScaleHorizontal( void ) {
	return g_world.terrain.scaleHorizontal;
}

float G_WorldGetTerrainScaleVertical( void ) {
	return g_world.terrain.scaleVertical;
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

	// Set skybox from world definition
	{
		const char *skyboxShader = G_WorldGetSkybox();
		
		if ( skyboxShader && skyboxShader[0] ) {
			
			trap_Cvar_Set( "r_forceSky", skyboxShader );
		}
	}

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

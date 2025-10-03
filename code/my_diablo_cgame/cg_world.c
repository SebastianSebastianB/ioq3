#include "cg_local.h"

#define JSON_IMPLEMENTATION
#include "../qcommon/json.h"
#undef JSON_IMPLEMENTATION

// Minimal image loader for grayscale PGM (P5) heightmaps
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image_stub.h"
#undef STB_IMAGE_IMPLEMENTATION

#ifdef _WIN32
#include <windows.h>
#endif

#include "../renderer_terrain/rt_api.h"

#define MAX_WORLD_FILE_SIZE 65536

#define MAX_HEIGHTMAP_FILE_SIZE (8 * 1024 * 1024)
#define MAX_TERRAIN_DIMENSION 1024
#define MAX_TERRAIN_SAMPLES (MAX_TERRAIN_DIMENSION * MAX_TERRAIN_DIMENSION)
#define MAX_TERRAIN_QUADS ((MAX_TERRAIN_DIMENSION - 1) * (MAX_TERRAIN_DIMENSION - 1))

static byte s_heightmapFileBuffer[MAX_HEIGHTMAP_FILE_SIZE];
static float s_heightSamples[MAX_TERRAIN_SAMPLES];
static polyVert_t s_terrainQuads[MAX_TERRAIN_QUADS * 4];

#ifdef _WIN32
static HMODULE s_rtLib = NULL;
#endif
static RT_Handle* s_rt = NULL;

typedef RT_Handle* (*PFN_RT_CreateFromHeights)(const float*, int, int, float, float);
typedef void (*PFN_RT_Destroy)(RT_Handle*);
typedef void (*PFN_RT_SetCamera)(RT_Handle*, const RT_Camera*);
typedef void (*PFN_RT_Render)(RT_Handle*, RT_EmitQuadFn, void*);

static PFN_RT_CreateFromHeights pRT_CreateFromHeights = NULL;
static PFN_RT_Destroy          pRT_Destroy = NULL;
static PFN_RT_SetCamera        pRT_SetCamera = NULL;
static PFN_RT_Render           pRT_Render = NULL;

static void CG_RT_Unload(void)
{
	if (s_rt && pRT_Destroy) {
		pRT_Destroy(s_rt);
	}
	s_rt = NULL;
#ifdef _WIN32
	if (s_rtLib) {
		FreeLibrary(s_rtLib);
		s_rtLib = NULL;
	}
#endif
	pRT_CreateFromHeights = NULL;
	pRT_Destroy = NULL;
	pRT_SetCamera = NULL;
	pRT_Render = NULL;
}

static qboolean CG_RT_Load(void)
{
	if (pRT_CreateFromHeights && pRT_Render) return qtrue;
#ifdef _WIN32
	s_rtLib = LoadLibraryA("renderer_terrain.dll");
	if (!s_rtLib) {
		s_rtLib = LoadLibraryA("my_diablo_output\\renderer_terrain.dll");
	}
	if (!s_rtLib) {
		CG_Printf(S_COLOR_YELLOW "WARNING: renderer_terrain.dll not found, fallback to simple mesh rendering\n");
		return qfalse;
	}
	pRT_CreateFromHeights = (PFN_RT_CreateFromHeights)GetProcAddress(s_rtLib, "RT_CreateFromHeights");
	pRT_Destroy           = (PFN_RT_Destroy)         GetProcAddress(s_rtLib, "RT_Destroy");
	pRT_SetCamera         = (PFN_RT_SetCamera)       GetProcAddress(s_rtLib, "RT_SetCamera");
	pRT_Render            = (PFN_RT_Render)          GetProcAddress(s_rtLib, "RT_Render");
	if (!pRT_CreateFromHeights || !pRT_Destroy || !pRT_SetCamera || !pRT_Render) {
		CG_Printf(S_COLOR_YELLOW "WARNING: renderer_terrain.dll missing required exports\n");
		CG_RT_Unload();
		return qfalse;
	}
	return qtrue;
#else
	return qfalse;
#endif
}

static void Q_to_RT_Camera(RT_Camera* out)
{
	vec3_t org;
	VectorCopy(cg.refdef.vieworg, org);
	out->origin.x = org[0];
	out->origin.y = org[1];
	out->origin.z = org[2];
	out->forward.x = cg.refdef.viewaxis[0][0];
	out->forward.y = cg.refdef.viewaxis[0][1];
	out->forward.z = cg.refdef.viewaxis[0][2];
	out->right.x   = cg.refdef.viewaxis[1][0];
	out->right.y   = cg.refdef.viewaxis[1][1];
	out->right.z   = cg.refdef.viewaxis[1][2];
	out->up.x      = cg.refdef.viewaxis[2][0];
	out->up.y      = cg.refdef.viewaxis[2][1];
	out->up.z      = cg.refdef.viewaxis[2][2];
	out->fovX = cg.refdef.fov_x;
	out->fovY = cg.refdef.fov_y;
	out->znear = 4.0f;
	out->zfar  = 8192.0f;
}

static void CG_RT_EmitQuad(const RT_Vert v[4], void* user)
{
	qhandle_t shader = (qhandle_t)(intptr_t)user;
	polyVert_t quad[4];
	for (int i = 0; i < 4; ++i) {
		quad[i].xyz[0] = v[i].xyz[0];
		quad[i].xyz[1] = v[i].xyz[1];
		quad[i].xyz[2] = v[i].xyz[2];
		quad[i].st[0] = v[i].st[0];
		quad[i].st[1] = v[i].st[1];
		quad[i].modulate[0] = 255;
		quad[i].modulate[1] = 255;
		quad[i].modulate[2] = 255;
		quad[i].modulate[3] = 255;
	}
	trap_R_AddPolysToScene(shader, 4, quad, 1);
}

static qboolean CG_ParseWorldVec3(const char *json, const char *jsonEnd, vec3_t out)
{
	const char *components[3] = { NULL, NULL, NULL };
	int count;

	if (!json || JSON_ValueGetType(json, jsonEnd) != JSONTYPE_ARRAY)
	{
		return qfalse;
	}

	count = JSON_ArrayGetIndex(json, jsonEnd, components, 3);
	if (count < 3)
	{
		return qfalse;
	}

	for (int i = 0; i < 3; ++i)
	{
		if (!components[i])
		{
			return qfalse;
		}

		out[i] = JSON_ValueGetFloat(components[i], jsonEnd);
	}

	return qtrue;
}

void CG_ClearWorldDefinition(void)
{
	CG_RT_Unload();
	memset(&cg.world, 0, sizeof(cg.world));
}

static void CG_SetWorldDefaults(void)
{
	cg.world.terrain.scaleHorizontal = 1.0f;
	cg.world.terrain.scaleVertical = 1.0f;
}

static void CG_ParseWorldTerrain(const char *terrainJson, const char *jsonEnd)
{
	const char *value;

	if (!terrainJson || JSON_ValueGetType(terrainJson, jsonEnd) != JSONTYPE_OBJECT)
	{
		return;
	}

	cg.world.terrain.enabled = qtrue;

	value = JSON_ObjectGetNamedValue(terrainJson, jsonEnd, "heightmap");
	if (value && JSON_ValueGetString(value, jsonEnd, cg.world.terrain.heightmap, sizeof(cg.world.terrain.heightmap)))
	{
		// value copied
	}

	value = JSON_ObjectGetNamedValue(terrainJson, jsonEnd, "splatmap");
	if (value && JSON_ValueGetString(value, jsonEnd, cg.world.terrain.splatmap, sizeof(cg.world.terrain.splatmap)))
	{
		// value copied
	}

	value = JSON_ObjectGetNamedValue(terrainJson, jsonEnd, "scale");
	if (value && JSON_ValueGetType(value, jsonEnd) == JSONTYPE_OBJECT)
	{
		const char *scaleValue;

		scaleValue = JSON_ObjectGetNamedValue(value, jsonEnd, "horizontal");
		if (scaleValue)
		{
			cg.world.terrain.scaleHorizontal = JSON_ValueGetFloat(scaleValue, jsonEnd);
			cg.world.terrain.hasScale = qtrue;
		}

		scaleValue = JSON_ObjectGetNamedValue(value, jsonEnd, "vertical");
		if (scaleValue)
		{
			cg.world.terrain.scaleVertical = JSON_ValueGetFloat(scaleValue, jsonEnd);
			cg.world.terrain.hasScale = qtrue;
		}
	}

	value = JSON_ObjectGetNamedValue(terrainJson, jsonEnd, "textures");
	if (value && JSON_ValueGetType(value, jsonEnd) == JSONTYPE_OBJECT)
	{
		const char *textureValue;

		textureValue = JSON_ObjectGetNamedValue(value, jsonEnd, "base");
		if (textureValue)
		{
			JSON_ValueGetString(textureValue, jsonEnd, cg.world.terrain.baseTexture, sizeof(cg.world.terrain.baseTexture));
		}

		textureValue = JSON_ObjectGetNamedValue(value, jsonEnd, "red");
		if (textureValue)
		{
			JSON_ValueGetString(textureValue, jsonEnd, cg.world.terrain.redTexture, sizeof(cg.world.terrain.redTexture));
		}

		textureValue = JSON_ObjectGetNamedValue(value, jsonEnd, "green");
		if (textureValue)
		{
			JSON_ValueGetString(textureValue, jsonEnd, cg.world.terrain.greenTexture, sizeof(cg.world.terrain.greenTexture));
		}

		textureValue = JSON_ObjectGetNamedValue(value, jsonEnd, "blue");
		if (textureValue)
		{
			JSON_ValueGetString(textureValue, jsonEnd, cg.world.terrain.blueTexture, sizeof(cg.world.terrain.blueTexture));
		}
	}
}

static void CG_ParseWorldLighting(const char *lightingJson, const char *jsonEnd)
{
	if (!lightingJson || JSON_ValueGetType(lightingJson, jsonEnd) != JSONTYPE_OBJECT)
	{
		return;
	}

	const char *value = JSON_ObjectGetNamedValue(lightingJson, jsonEnd, "sunDirection");
	if (CG_ParseWorldVec3(value, jsonEnd, cg.world.lighting.sunDirection))
	{
		cg.world.lighting.hasSunDirection = qtrue;
	}

	value = JSON_ObjectGetNamedValue(lightingJson, jsonEnd, "sunColor");
	if (CG_ParseWorldVec3(value, jsonEnd, cg.world.lighting.sunColor))
	{
		cg.world.lighting.hasSunColor = qtrue;
	}

	value = JSON_ObjectGetNamedValue(lightingJson, jsonEnd, "ambientColor");
	if (CG_ParseWorldVec3(value, jsonEnd, cg.world.lighting.ambientColor))
	{
		cg.world.lighting.hasAmbientColor = qtrue;
	}
}

static qhandle_t CG_TryRegisterTerrainShader(const char *path, const char *label)
{
	qhandle_t shader;

	if (!path[0])
	{
		return 0;
	}

	shader = trap_R_RegisterShader(path);
	if (!shader)
	{
		CG_Printf(S_COLOR_YELLOW "WARNING: failed to register %s '%s'\n", label, path);
	}

	return shader;
}

static void CG_RegisterTerrainTextures(void)
{
	cg.world.terrain.baseShader = CG_TryRegisterTerrainShader(cg.world.terrain.baseTexture, "terrain base texture");
	cg.world.terrain.redShader = CG_TryRegisterTerrainShader(cg.world.terrain.redTexture, "terrain red texture");
	cg.world.terrain.greenShader = CG_TryRegisterTerrainShader(cg.world.terrain.greenTexture, "terrain green texture");
	cg.world.terrain.blueShader = CG_TryRegisterTerrainShader(cg.world.terrain.blueTexture, "terrain blue texture");

	if (cg.world.terrain.splatmap[0])
	{
		cg.world.terrain.splatmapShader = trap_R_RegisterShaderNoMip(cg.world.terrain.splatmap);
		if (!cg.world.terrain.splatmapShader)
		{
			CG_Printf(S_COLOR_YELLOW "WARNING: failed to register terrain splatmap '%s'\n", cg.world.terrain.splatmap);
		}
	}
}

static qboolean CG_LoadTerrainHeightmap(void)
{
	fileHandle_t file;
	int fileLength;
	int width = 0, height = 0, comp = 0;
	stbi_uc *pixels;
	int sampleCount;

	if (!cg.world.terrain.heightmap[0])
	{
		CG_Printf(S_COLOR_YELLOW "WARNING: world definition is missing terrain.heightmap\n");
		return qfalse;
	}

	fileLength = trap_FS_FOpenFile(cg.world.terrain.heightmap, &file, FS_READ);
	if (fileLength <= 0 || !file)
	{
		CG_Error("Failed to open terrain heightmap '%s'", cg.world.terrain.heightmap);
	}

	if (fileLength > MAX_HEIGHTMAP_FILE_SIZE)
	{
		trap_FS_FCloseFile(file);
		CG_Error("Terrain heightmap '%s' exceeds maximum supported size (%d bytes)", cg.world.terrain.heightmap, MAX_HEIGHTMAP_FILE_SIZE);
	}

	trap_FS_Read(s_heightmapFileBuffer, fileLength, file);
	trap_FS_FCloseFile(file);

	stbi_set_flip_vertically_on_load(1);
	pixels = stbi_load_from_memory(s_heightmapFileBuffer, fileLength, &width, &height, &comp, 1);
	if (!pixels)
	{
		CG_Error("Failed to decode terrain heightmap '%s'", cg.world.terrain.heightmap);
	}

	if (width < 2 || height < 2)
	{
		stbi_image_free(pixels);
		CG_Error("Terrain heightmap '%s' must be at least 2x2 pixels", cg.world.terrain.heightmap);
	}

	if (width > MAX_TERRAIN_DIMENSION || height > MAX_TERRAIN_DIMENSION)
	{
		stbi_image_free(pixels);
		CG_Error("Terrain heightmap '%s' exceeds maximum supported dimension %d", cg.world.terrain.heightmap, MAX_TERRAIN_DIMENSION);
	}

	cg.world.terrain.heightmapWidth = width;
	cg.world.terrain.heightmapHeight = height;
	cg.world.terrain.heightSamples = s_heightSamples;

	sampleCount = width * height;
	for (int i = 0; i < sampleCount; ++i)
	{
		float normalized = pixels[i] / 255.0f;
		// Store NORMALIZED values (0.0-1.0), NOT scaled! Match RT_CreateFromHeights behavior.
		// Scaling happens later in CG_Trace (multiply by scaleV), just like RT_GetHeightAt does.
		s_heightSamples[i] = normalized;
	}

	stbi_image_free(pixels);
	return qtrue;
}

static void CG_BuildTerrainMesh(void)
{
	const int width = cg.world.terrain.heightmapWidth;
	const int height = cg.world.terrain.heightmapHeight;
	const float step = cg.world.terrain.scaleHorizontal;
	const float scaleV = cg.world.terrain.scaleVertical;
	// IMPORTANT: Match server terrain coordinate system (starts at 0,0 not centered)
	// const float halfWidth = (width - 1) * step * 0.5f;
	// const float halfHeight = (height - 1) * step * 0.5f;
	int quadIndex = 0;

	CG_Printf("^3[BUILD MESH] width=%d height=%d scaleH=%.2f scaleV=%.2f\n", width, height, step, scaleV);

	if (width < 2 || height < 2)
	{
		CG_Printf("^1[BUILD MESH] ERROR: Invalid dimensions!\n");
		cg.world.terrain.meshVerts = NULL;
		cg.world.terrain.meshVertCount = 0;
		cg.world.terrain.meshQuadCount = 0;
		return;
	}

	for (int y = 0; y < height - 1; ++y)
	{
		for (int x = 0; x < width - 1; ++x)
		{
			polyVert_t *quad = &s_terrainQuads[quadIndex * 4];
			int idxBL = y * width + x;
			int idxBR = y * width + (x + 1);
			int idxTR = (y + 1) * width + (x + 1);
			int idxTL = (y + 1) * width + x;

			// Use same coordinate system as server: terrain starts at (0,0)
			float worldX0 = x * step;
			float worldX1 = (x + 1) * step;
			float worldY0 = y * step;
			float worldY1 = (y + 1) * step;

			// FIX: Use counter-clockwise winding order: BL → TL → TR → BR
			polyVert_t *vBL = &quad[0];  // Bottom-Left
			polyVert_t *vTL = &quad[1];  // Top-Left
			polyVert_t *vTR = &quad[2];  // Top-Right
			polyVert_t *vBR = &quad[3];  // Bottom-Right

			vBL->xyz[0] = worldX0;
			vBL->xyz[1] = worldY0;
			vBL->xyz[2] = s_heightSamples[idxBL] * cg.world.terrain.scaleVertical;
			vBL->st[0] = (float)x / (float)(width - 1);
			vBL->st[1] = (float)y / (float)(height - 1);

			vTL->xyz[0] = worldX0;
			vTL->xyz[1] = worldY1;
			vTL->xyz[2] = s_heightSamples[idxTL] * cg.world.terrain.scaleVertical;
			vTL->st[0] = (float)x / (float)(width - 1);
			vTL->st[1] = (float)(y + 1) / (float)(height - 1);

			vTR->xyz[0] = worldX1;
			vTR->xyz[1] = worldY1;
			vTR->xyz[2] = s_heightSamples[idxTR] * cg.world.terrain.scaleVertical;
			vTR->st[0] = (float)(x + 1) / (float)(width - 1);
			vTR->st[1] = (float)(y + 1) / (float)(height - 1);

			vBR->xyz[0] = worldX1;
			vBR->xyz[1] = worldY0;
			vBR->xyz[2] = s_heightSamples[idxBR] * cg.world.terrain.scaleVertical;
			vBR->st[0] = (float)(x + 1) / (float)(width - 1);
			vBR->st[1] = (float)y / (float)(height - 1);

			for (int i = 0; i < 4; ++i)
			{
				quad[i].modulate[0] = 255;
				quad[i].modulate[1] = 255;
				quad[i].modulate[2] = 255;
				quad[i].modulate[3] = 255;
			}

			quadIndex++;
		}
	}

	cg.world.terrain.meshVerts = s_terrainQuads;
	cg.world.terrain.meshVertCount = quadIndex * 4;
	cg.world.terrain.meshQuadCount = quadIndex;
	
	// Log first and last quad coordinates
	int lastQuadIdx = (quadIndex - 1) * 4;
	CG_Printf("^2[BUILD MESH] SUCCESS: Built %d quads (%d verts)\n", quadIndex, quadIndex * 4);
	CG_Printf("^3[BUILD MESH] First quad: (%.1f,%.1f,%.1f) to (%.1f,%.1f,%.1f)\n", 
		s_terrainQuads[0].xyz[0], s_terrainQuads[0].xyz[1], s_terrainQuads[0].xyz[2],
		s_terrainQuads[2].xyz[0], s_terrainQuads[2].xyz[1], s_terrainQuads[2].xyz[2]);
	CG_Printf("^3[BUILD MESH] Last quad: (%.1f,%.1f,%.1f) to (%.1f,%.1f,%.1f)\n", 
		s_terrainQuads[lastQuadIdx].xyz[0], s_terrainQuads[lastQuadIdx].xyz[1], s_terrainQuads[lastQuadIdx].xyz[2],
		s_terrainQuads[lastQuadIdx+2].xyz[0], s_terrainQuads[lastQuadIdx+2].xyz[1], s_terrainQuads[lastQuadIdx+2].xyz[2]);
}

static void CG_LoadTerrainAssets(void)
{
	if (!cg.world.terrain.enabled)
	{
		
		return;
	}

	
	if (!CG_LoadTerrainHeightmap())
	{
		
		cg.world.terrain.enabled = qfalse;
		return;
	}

	// Try external renderer first
	if (CG_RT_Load()) {
		s_rt = pRT_CreateFromHeights(s_heightSamples,
			cg.world.terrain.heightmapWidth,
			cg.world.terrain.heightmapHeight,
			cg.world.terrain.scaleHorizontal,
			1.0f /* heights already scaled */);
		if (!s_rt) {
			CG_Printf(S_COLOR_YELLOW "WARNING: RT_CreateFromHeights failed, falling back to mesh\n");
		}
	}

	
	CG_BuildTerrainMesh();
	
	CG_RegisterTerrainTextures();
	cg.world.terrain.resourcesLoaded = (cg.world.terrain.meshQuadCount > 0);
	
}

void CG_AddTerrainToScene(void)
{
	qhandle_t shader;

	CG_Printf("^5[RENDER] CG_AddTerrainToScene called - active=%d, resourcesLoaded=%d, meshQuadCount=%d\n", 
		cg.world.active, cg.world.terrain.resourcesLoaded, cg.world.terrain.meshQuadCount);

	if (!cg.world.active || !cg.world.terrain.resourcesLoaded || cg.world.terrain.meshQuadCount <= 0)
	{
		CG_Printf("^1[RENDER] Early return - active=%d, resourcesLoaded=%d, meshQuadCount=%d\n", 
			cg.world.active, cg.world.terrain.resourcesLoaded, cg.world.terrain.meshQuadCount);
		return;
	}

	if (!cg.world.terrain.meshVerts)
	{
		CG_Printf("^1[RENDER] Early return - meshVerts is NULL\n");
		return;
	}

	CG_Printf("^2[RENDER] Rendering %d quads\n", cg.world.terrain.meshQuadCount);

	shader = cg.world.terrain.baseShader;
	if (!shader)
	{
		shader = cgs.media.whiteShader;
		CG_Printf("^3[RENDER] Using whiteShader\n");
	} else {
		CG_Printf("^2[RENDER] Using baseShader: %d\n", shader);
	}

	// TEMPORARILY DISABLE RT RENDERER - use direct mesh rendering
	if (0 && s_rt && pRT_SetCamera && pRT_Render) {
		RT_Camera cam;
		Q_to_RT_Camera(&cam);
		pRT_SetCamera(s_rt, &cam);
		pRT_Render(s_rt, CG_RT_EmitQuad, (void*)(intptr_t)shader);
		
		return;
	}

	CG_Printf("^4[RENDER] About to call trap_R_AddPolysToScene - shader=%d, quadCount=%d\n", 
		shader, cg.world.terrain.meshQuadCount);
	trap_R_AddPolysToScene(shader, 4, cg.world.terrain.meshVerts, cg.world.terrain.meshQuadCount);
	
}

qboolean CG_LoadWorldDefinition(const char *mapName)
{
	fileHandle_t handle;
	int length;
	static char buffer[MAX_WORLD_FILE_SIZE];
	const char *jsonEnd;
	const char *value;

	if (!mapName || !COM_CompareExtension(mapName, ".world"))
	{
		CG_ClearWorldDefinition();
		return qfalse;
	}

	CG_ClearWorldDefinition();
	CG_SetWorldDefaults();

	Q_strncpyz(cg.world.fileName, mapName, sizeof(cg.world.fileName));

	length = trap_FS_FOpenFile(mapName, &handle, FS_READ);
	if (length <= 0 || !handle)
	{
		CG_Error("Failed to open world file '%s'", mapName);
	}

	if (length >= MAX_WORLD_FILE_SIZE)
	{
		trap_FS_FCloseFile(handle);
		CG_Error("World file '%s' exceeds maximum supported size (%d bytes)", mapName, MAX_WORLD_FILE_SIZE - 1);
	}

	trap_FS_Read(buffer, length, handle);
	buffer[length] = '\0';
	trap_FS_FCloseFile(handle);

	jsonEnd = buffer + length;

	if (JSON_ValueGetType(buffer, jsonEnd) != JSONTYPE_OBJECT)
	{
		CG_Error("World file '%s' must begin with a JSON object", mapName);
	}

	value = JSON_ObjectGetNamedValue(buffer, jsonEnd, "worldName");
	if (value)
	{
		JSON_ValueGetString(value, jsonEnd, cg.world.worldName, sizeof(cg.world.worldName));
	}

	value = JSON_ObjectGetNamedValue(buffer, jsonEnd, "version");
	if (value)
	{
		cg.world.version = JSON_ValueGetInt(value, jsonEnd);
	}

	value = JSON_ObjectGetNamedValue(buffer, jsonEnd, "skybox");
	if (value && JSON_ValueGetString(value, jsonEnd, cg.world.skybox, sizeof(cg.world.skybox)))
	{
		cg.world.hasSkybox = qtrue;
	}

	CG_ParseWorldTerrain(JSON_ObjectGetNamedValue(buffer, jsonEnd, "terrain"), jsonEnd);
	CG_ParseWorldLighting(JSON_ObjectGetNamedValue(buffer, jsonEnd, "lighting"), jsonEnd);

	value = JSON_ObjectGetNamedValue(buffer, jsonEnd, "objects");
	if (value && JSON_ValueGetString(value, jsonEnd, cg.world.objectsFile, sizeof(cg.world.objectsFile)))
	{
		cg.world.hasObjectsFile = qtrue;
	}

	if (!cg.world.worldName[0])
	{
		Q_strncpyz(cg.world.worldName, cg.world.fileName, sizeof(cg.world.worldName));
	}

	
	CG_LoadTerrainAssets();
	

	cg.world.active = qtrue;

	CG_Printf(S_COLOR_GREEN "Loaded world definition: %s\n", cg.world.worldName);

	return qtrue;
}

/*
=================
CG_DrawWorldSkybox

Renders skybox for world maps that don't have BSP world model.
Draws a large cube around the camera using the skybox shader from world definition.
=================
*/
void CG_DrawWorldSkybox(void) {
	polyVert_t verts[4];
	vec3_t skyboxSize;
	float size = 8000.0f; // Very large cube around player
	qhandle_t skyShader;
	vec3_t origin;
	int i;
	
	// Only draw skybox if we have terrain enabled (world map)
	if (!cg.world.terrain.enabled) {
		return;
	}
	
	// Use skybox shader from server (set via r_forceSky cvar)
	// For now, register hellsky directly as fallback
	skyShader = trap_R_RegisterShader("textures/skies/hellsky");
	if (!skyShader) {
		return; // No skybox shader available
	}
	
	// Center skybox on camera position
	VectorCopy(cg.refdef.vieworg, origin);
	
	// Draw 6 faces of skybox cube
	// All vertices use same texture coordinates (skybox shader handles this)
	
	// Face 1: Front (+Y)
	verts[0].modulate[0] = verts[0].modulate[1] = verts[0].modulate[2] = verts[0].modulate[3] = 255;
	verts[0].st[0] = 0; verts[0].st[1] = 0;
	verts[0].xyz[0] = origin[0] - size; verts[0].xyz[1] = origin[1] + size; verts[0].xyz[2] = origin[2] - size;
	
	verts[1].st[0] = 1; verts[1].st[1] = 0;
	verts[1].xyz[0] = origin[0] + size; verts[1].xyz[1] = origin[1] + size; verts[1].xyz[2] = origin[2] - size;
	verts[1].modulate[0] = verts[1].modulate[1] = verts[1].modulate[2] = verts[1].modulate[3] = 255;
	
	verts[2].st[0] = 1; verts[2].st[1] = 1;
	verts[2].xyz[0] = origin[0] + size; verts[2].xyz[1] = origin[1] + size; verts[2].xyz[2] = origin[2] + size;
	verts[2].modulate[0] = verts[2].modulate[1] = verts[2].modulate[2] = verts[2].modulate[3] = 255;
	
	verts[3].st[0] = 0; verts[3].st[1] = 1;
	verts[3].xyz[0] = origin[0] - size; verts[3].xyz[1] = origin[1] + size; verts[3].xyz[2] = origin[2] + size;
	verts[3].modulate[0] = verts[3].modulate[1] = verts[3].modulate[2] = verts[3].modulate[3] = 255;
	
	trap_R_AddPolyToScene(skyShader, 4, verts);
	
	// Face 2: Back (-Y)
	verts[0].xyz[0] = origin[0] + size; verts[0].xyz[1] = origin[1] - size; verts[0].xyz[2] = origin[2] - size;
	verts[1].xyz[0] = origin[0] - size; verts[1].xyz[1] = origin[1] - size; verts[1].xyz[2] = origin[2] - size;
	verts[2].xyz[0] = origin[0] - size; verts[2].xyz[1] = origin[1] - size; verts[2].xyz[2] = origin[2] + size;
	verts[3].xyz[0] = origin[0] + size; verts[3].xyz[1] = origin[1] - size; verts[3].xyz[2] = origin[2] + size;
	trap_R_AddPolyToScene(skyShader, 4, verts);
	
	// Face 3: Left (-X)
	verts[0].xyz[0] = origin[0] - size; verts[0].xyz[1] = origin[1] - size; verts[0].xyz[2] = origin[2] - size;
	verts[1].xyz[0] = origin[0] - size; verts[1].xyz[1] = origin[1] + size; verts[1].xyz[2] = origin[2] - size;
	verts[2].xyz[0] = origin[0] - size; verts[2].xyz[1] = origin[1] + size; verts[2].xyz[2] = origin[2] + size;
	verts[3].xyz[0] = origin[0] - size; verts[3].xyz[1] = origin[1] - size; verts[3].xyz[2] = origin[2] + size;
	trap_R_AddPolyToScene(skyShader, 4, verts);
	
	// Face 4: Right (+X)
	verts[0].xyz[0] = origin[0] + size; verts[0].xyz[1] = origin[1] + size; verts[0].xyz[2] = origin[2] - size;
	verts[1].xyz[0] = origin[0] + size; verts[1].xyz[1] = origin[1] - size; verts[1].xyz[2] = origin[2] - size;
	verts[2].xyz[0] = origin[0] + size; verts[2].xyz[1] = origin[1] - size; verts[2].xyz[2] = origin[2] + size;
	verts[3].xyz[0] = origin[0] + size; verts[3].xyz[1] = origin[1] + size; verts[3].xyz[2] = origin[2] + size;
	trap_R_AddPolyToScene(skyShader, 4, verts);
	
	// Face 5: Top (+Z)
	verts[0].xyz[0] = origin[0] - size; verts[0].xyz[1] = origin[1] - size; verts[0].xyz[2] = origin[2] + size;
	verts[1].xyz[0] = origin[0] - size; verts[1].xyz[1] = origin[1] + size; verts[1].xyz[2] = origin[2] + size;
	verts[2].xyz[0] = origin[0] + size; verts[2].xyz[1] = origin[1] + size; verts[2].xyz[2] = origin[2] + size;
	verts[3].xyz[0] = origin[0] + size; verts[3].xyz[1] = origin[1] - size; verts[3].xyz[2] = origin[2] + size;
	trap_R_AddPolyToScene(skyShader, 4, verts);
	
	// Face 6: Bottom (-Z)
	verts[0].xyz[0] = origin[0] - size; verts[0].xyz[1] = origin[1] + size; verts[0].xyz[2] = origin[2] - size;
	verts[1].xyz[0] = origin[0] - size; verts[1].xyz[1] = origin[1] - size; verts[1].xyz[2] = origin[2] - size;
	verts[2].xyz[0] = origin[0] + size; verts[2].xyz[1] = origin[1] - size; verts[2].xyz[2] = origin[2] - size;
	verts[3].xyz[0] = origin[0] + size; verts[3].xyz[1] = origin[1] + size; verts[3].xyz[2] = origin[2] - size;
	trap_R_AddPolyToScene(skyShader, 4, verts);
}

#include "cg_local.h"

#define JSON_IMPLEMENTATION
#include "../qcommon/json.h"
#undef JSON_IMPLEMENTATION

#define STB_IMAGE_IMPLEMENTATION
#include "../../thirdparty/stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#define MAX_WORLD_FILE_SIZE 65536

#define MAX_HEIGHTMAP_FILE_SIZE (8 * 1024 * 1024)
#define MAX_TERRAIN_DIMENSION 1024
#define MAX_TERRAIN_SAMPLES (MAX_TERRAIN_DIMENSION * MAX_TERRAIN_DIMENSION)
#define MAX_TERRAIN_QUADS ((MAX_TERRAIN_DIMENSION - 1) * (MAX_TERRAIN_DIMENSION - 1))

static byte s_heightmapFileBuffer[MAX_HEIGHTMAP_FILE_SIZE];
static float s_heightSamples[MAX_TERRAIN_SAMPLES];
static polyVert_t s_terrainQuads[MAX_TERRAIN_QUADS * 4];

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
		s_heightSamples[i] = normalized * cg.world.terrain.scaleVertical;
	}

	stbi_image_free(pixels);
	return qtrue;
}

static void CG_BuildTerrainMesh(void)
{
	const int width = cg.world.terrain.heightmapWidth;
	const int height = cg.world.terrain.heightmapHeight;
	const float step = cg.world.terrain.scaleHorizontal;
	const float halfWidth = (width - 1) * step * 0.5f;
	const float halfHeight = (height - 1) * step * 0.5f;
	int quadIndex = 0;

	if (width < 2 || height < 2)
	{
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

			float worldX0 = x * step - halfWidth;
			float worldX1 = (x + 1) * step - halfWidth;
			float worldY0 = y * step - halfHeight;
			float worldY1 = (y + 1) * step - halfHeight;

			polyVert_t *vBL = &quad[0];
			polyVert_t *vBR = &quad[1];
			polyVert_t *vTR = &quad[2];
			polyVert_t *vTL = &quad[3];

			vBL->xyz[0] = worldX0;
			vBL->xyz[1] = worldY0;
			vBL->xyz[2] = s_heightSamples[idxBL];
			vBL->st[0] = (float)x / (float)(width - 1);
			vBL->st[1] = (float)y / (float)(height - 1);

			vBR->xyz[0] = worldX1;
			vBR->xyz[1] = worldY0;
			vBR->xyz[2] = s_heightSamples[idxBR];
			vBR->st[0] = (float)(x + 1) / (float)(width - 1);
			vBR->st[1] = (float)y / (float)(height - 1);

			vTR->xyz[0] = worldX1;
			vTR->xyz[1] = worldY1;
			vTR->xyz[2] = s_heightSamples[idxTR];
			vTR->st[0] = (float)(x + 1) / (float)(width - 1);
			vTR->st[1] = (float)(y + 1) / (float)(height - 1);

			vTL->xyz[0] = worldX0;
			vTL->xyz[1] = worldY1;
			vTL->xyz[2] = s_heightSamples[idxTL];
			vTL->st[0] = (float)x / (float)(width - 1);
			vTL->st[1] = (float)(y + 1) / (float)(height - 1);

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
}

static void CG_LoadTerrainAssets(void)
{
	if (!cg.world.terrain.enabled)
	{
		CG_Printf(S_COLOR_CYAN "DEBUG: Terrain not enabled, skipping asset loading\n");
		return;
	}

	CG_Printf(S_COLOR_CYAN "DEBUG: Loading terrain heightmap...\n");
	if (!CG_LoadTerrainHeightmap())
	{
		CG_Printf(S_COLOR_CYAN "DEBUG: Failed to load heightmap, disabling terrain\n");
		cg.world.terrain.enabled = qfalse;
		return;
	}

	CG_Printf(S_COLOR_CYAN "DEBUG: Building terrain mesh...\n");
	CG_BuildTerrainMesh();
	CG_Printf(S_COLOR_CYAN "DEBUG: Registering terrain textures...\n");
	CG_RegisterTerrainTextures();
	cg.world.terrain.resourcesLoaded = (cg.world.terrain.meshQuadCount > 0);
	CG_Printf(S_COLOR_CYAN "DEBUG: Terrain assets loaded, resourcesLoaded=%d\n", cg.world.terrain.resourcesLoaded);
}

void CG_AddTerrainToScene(void)
{
	qhandle_t shader;

	CG_Printf("DEBUG: CG_AddTerrainToScene entry\n");

	if (!cg.world.active || !cg.world.terrain.resourcesLoaded || cg.world.terrain.meshQuadCount <= 0)
	{
		CG_Printf("DEBUG: Early return - active=%d, resourcesLoaded=%d, meshQuadCount=%d\n", 
			cg.world.active, cg.world.terrain.resourcesLoaded, cg.world.terrain.meshQuadCount);
		return;
	}

	if (!cg.world.terrain.meshVerts)
	{
		CG_Printf("DEBUG: Early return - meshVerts is NULL\n");
		return;
	}

	shader = cg.world.terrain.baseShader;
	if (!shader)
	{
		shader = cgs.media.whiteShader;
		CG_Printf("DEBUG: Using fallback whiteShader\n");
	} else {
		CG_Printf("DEBUG: Using terrain baseShader\n");
	}

	CG_Printf("DEBUG: About to call trap_R_AddPolysToScene - shader=%d, quadCount=%d\n", 
		shader, cg.world.terrain.meshQuadCount);

	trap_R_AddPolysToScene(shader, 4, cg.world.terrain.meshVerts, cg.world.terrain.meshQuadCount);

	CG_Printf("DEBUG: trap_R_AddPolysToScene completed successfully\n");
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

	CG_Printf(S_COLOR_CYAN "DEBUG: About to load terrain assets, terrain.enabled=%d\n", cg.world.terrain.enabled);
	CG_LoadTerrainAssets();
	CG_Printf(S_COLOR_CYAN "DEBUG: Finished loading terrain assets\n");

	cg.world.active = qtrue;

	CG_Printf(S_COLOR_GREEN "Loaded world definition: %s\n", cg.world.worldName);

	return qtrue;
}

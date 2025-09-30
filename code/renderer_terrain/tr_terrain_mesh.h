#ifndef TR_TERRAIN_MESH_H
#define TR_TERRAIN_MESH_H

typedef struct {
    int quadCount;
} RT_Mesh;

void RT_BuildGridMesh(int width, int height, float step, RT_Mesh* out);

#endif

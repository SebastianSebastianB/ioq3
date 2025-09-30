#include "tr_terrain_mesh.h"
#include <string.h>

void RT_BuildGridMesh(int width, int height, float step, RT_Mesh* out) {
    if (!out) return;
    (void)step; // currently unused
    memset(out, 0, sizeof(*out));
    if (width < 2 || height < 2) return;
    out->quadCount = (width - 1) * (height - 1);
}

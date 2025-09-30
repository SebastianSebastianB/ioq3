// Very small subset of stb_image API to load grayscale PGM (P5) images from memory.
// This keeps changes inside the mod tree and avoids adding thirdparty deps.
#ifndef STB_IMAGE_STUB_H
#define STB_IMAGE_STUB_H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef unsigned char stbi_uc;

static int s_stbi_flip_vertical = 0;
static void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip) { s_stbi_flip_vertical = flag_true_if_should_flip; }

static void pgm_skip_ws_and_comments(const unsigned char* data, int len, int* i) {
    while (*i < len) {
        if (isspace((int)data[*i])) { (*i)++; continue; }
        if (data[*i] == '#') { while (*i < len && data[*i] != '\n') { (*i)++; } continue; }
        break;
    }
}

static int pgm_parse_int(const unsigned char* data, int len, int* i) {
    int v = 0;
    int any = 0;
    while (*i < len && isdigit((int)data[*i])) { v = v*10 + (data[*i]-'0'); (*i)++; any = 1; }
    return any ? v : -1;
}

// Parse PGM (P5) header; returns pointer to pixel data start or NULL.
static const unsigned char* parse_pgm_header(const unsigned char* data, int len, int* w, int* h, int* maxv) {
    int i;
    int ww, hh, mm;
    if (len < 11) return NULL;
    if (!(data[0]=='P' && data[1]=='5')) return NULL;
    i = 2;
    pgm_skip_ws_and_comments(data, len, &i);
    ww = pgm_parse_int(data, len, &i);
    if (ww <= 0) return NULL;
    pgm_skip_ws_and_comments(data, len, &i);
    hh = pgm_parse_int(data, len, &i);
    if (hh <= 0) return NULL;
    pgm_skip_ws_and_comments(data, len, &i);
    mm = pgm_parse_int(data, len, &i);
    if (mm <= 0 || mm > 65535) return NULL;
    if (i >= len) return NULL;
    // one whitespace after maxval
    i++;
    *w = ww; *h = hh; *maxv = mm;
    return (i <= len) ? data + i : NULL;
}

static stbi_uc* stbi_load_from_memory(const unsigned char* buffer, int len, int* x, int* y, int* comp, int req_comp) {
    int w, h, maxv;
    const unsigned char* pixels;
    int bytes_per_sample;
    size_t need;
    stbi_uc* out;
    int row, col;
    (void)req_comp; // we always return 1 component

    pixels = parse_pgm_header(buffer, len, &w, &h, &maxv);
    if (!pixels) return NULL;

    bytes_per_sample = (maxv > 255) ? 2 : 1;
    need = (size_t)w * (size_t)h * (size_t)bytes_per_sample;
    if ((size_t)(pixels - buffer) + need > (size_t)len) return NULL;

    out = (stbi_uc*)malloc((size_t)w * (size_t)h);
    if (!out) return NULL;

    if (bytes_per_sample == 1) {
        memcpy(out, pixels, (size_t)w * (size_t)h);
    } else {
        // 16-bit big-endian per PGM spec: convert to 8-bit by >> 8
        int i;
        for (i = 0; i < w*h; ++i) {
            unsigned int v = ((unsigned int)pixels[i*2] << 8) | (unsigned int)pixels[i*2 + 1];
            out[i] = (unsigned char)(v >> 8);
        }
    }

    if (s_stbi_flip_vertical) {
        for (row = 0; row < h/2; ++row) {
            stbi_uc* a = out + row * w;
            stbi_uc* b = out + (h - 1 - row) * w;
            for (col = 0; col < w; ++col) {
                stbi_uc tmp = a[col]; a[col] = b[col]; b[col] = tmp;
            }
        }
    }

    if (x) *x = w; if (y) *y = h; if (comp) *comp = 1;
    return out;
}

static void stbi_image_free(void* data) {
    free(data);
}

#endif // STB_IMAGE_STUB_H

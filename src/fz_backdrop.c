/*
 * fz_backdrop.c -- Forza Motorsport's menu backgrounds, widened for 16:9.
 *
 * Every menu background is a linear A8R8G8B8 texture, 720x480 (or 640x480),
 * from the player's own disc (cabs\MegaXPR.ca2). The layouts squeeze its
 * texels into 640 screen pixels, so the 16:9 picture needs it 4/3 as wide at
 * the same density: 960 texels (853 for a 640-wide one). fz_layout.c widens
 * the controls that show them and their texture rectangles by that 4/3; this
 * makes the pictures to match, in memory, when the game loads one. Nothing is
 * shipped: the new columns are grown from the texture's own pixels.
 *
 * A recipe says how many new columns go on each side and how they are made
 * (layout/widescreen.txt, "texture" lines):
 *
 *   stripes  follow the panel's diagonal stripes into the new space: each new
 *            texel is the texel a stripe would carry there from a column just
 *            inside the art, stepping by the stripes' vertical repeat to stay
 *            inside the picture. Slope 1.2 (rising to the right) and a repeat
 *            of 18 texels, measured on Bkgd_FiberSwoosh; every striped
 *            background shares the pattern.
 *   clamp    repeat the edge column: right for flat panels, gradients and
 *            horizontal bars.
 *   clear    black, as opaque as the art's edge column: black sides for a
 *            picture, an open window left open in an overlay.
 *
 * A background with no recipe of its own gets "clear" on both sides, which
 * is the black sides the menus had before.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fz_backdrop.h"

#define FZ_STRIPE_SLOPE  1.2
#define FZ_STRIPE_REPEAT 18

/* Which rows at one edge are bands rather than stripes: a dark row that
 * hardly changes over the 160 columns inside the edge (the black bands and
 * grey bars a loading picture paints across itself; the white stripe panels
 * are light, and smooth in places, and must not count). A band row is
 * extended as it is; a stripe row borrows only from stripe rows in its own
 * run, so a stripe never drags a band into the art or the art into a band. */
#define FZ_BAND_COLUMNS 160
#define FZ_BAND_SPREAD  30      /* of R+G+B */
#define FZ_BAND_DARK    450     /* mean R+G+B under this */
#define FZ_MAX_ROWS     1024

static unsigned char g_band[FZ_MAX_ROWS];
static int g_band_side = -1;
static const uint8_t *g_band_src;

static int luma(const uint8_t *p)
{
    return p[0] + p[1] + p[2];
}

static void find_bands(const uint8_t *src, uint32_t w, uint32_t h, uint32_t pitch, int left)
{
    uint32_t y, x;

    if (g_band_src == src && g_band_side == left)
        return;
    g_band_src = src;
    g_band_side = left;
    for (y = 0; y < h && y < FZ_MAX_ROWS; y++) {
        int lo = 1 << 30, hi = -1, sum = 0, n = 0;

        for (x = 0; x < FZ_BAND_COLUMNS && x < w; x++, n++) {
            int l = luma(src + (size_t)y * pitch + (size_t)(left ? x : w - 1 - x) * 4u);
            if (l < lo) lo = l;
            if (l > hi) hi = l;
            sum += l;
        }
        g_band[y] = (unsigned char)(n && hi - lo < FZ_BAND_SPREAD && sum / n < FZ_BAND_DARK);
    }
}

static uint32_t sample_stripes(const uint8_t *src, uint32_t w, uint32_t h, uint32_t pitch,
                               int x_new, uint32_t y, int left)
{
    /* x_new < 0 left of the art, >= w right of it. Walk along the stripe
     * (rising to the right) to a column two texels inside the art, staying
     * in this row's run of stripe rows by the stripes' vertical repeat. */
    int edge = left ? 2 : (int)w - 3;
    double t = (double)(edge - x_new);          /* > 0 on the left, < 0 on the right */
    double ys = (double)y - FZ_STRIPE_SLOPE * t;
    uint32_t px, y0 = y, y1 = y + 1;

    find_bands(src, w, h, pitch, left);
    if (h > FZ_MAX_ROWS || g_band[y]) {
        memcpy(&px, src + (size_t)y * pitch + (size_t)(left ? 0 : w - 1) * 4u, 4);
        return px;
    }
    while (y0 > 0 && !g_band[y0 - 1])
        y0--;
    while (y1 < h && !g_band[y1])
        y1++;
    if (y1 - y0 <= FZ_STRIPE_REPEAT) {
        memcpy(&px, src + (size_t)y * pitch + (size_t)edge * 4u, 4);
        return px;
    }
    while (ys < (double)y0)
        ys += FZ_STRIPE_REPEAT;
    while (ys >= (double)y1)
        ys -= FZ_STRIPE_REPEAT;
    memcpy(&px, src + (size_t)(uint32_t)ys * pitch + (size_t)edge * 4u, 4);
    return px;
}

static uint32_t fill(FzFill mode, const uint8_t *src, uint32_t w, uint32_t h, uint32_t pitch,
                     int x_new, uint32_t y)
{
    int left = x_new < 0;
    uint32_t px = 0;

    switch (mode) {
    case FZ_FILL_STRIPES:
        return sample_stripes(src, w, h, pitch, x_new, y, left);
    case FZ_FILL_CLAMP:
        memcpy(&px, src + (size_t)y * pitch + (size_t)(left ? 0 : w - 1) * 4u, 4);
        return px;
    case FZ_FILL_CLEAR:
    default:
        /* Black, as opaque as the art's edge beside it: an opaque picture
         * gets opaque sides (transparent ones let the screen behind show
         * through during a cross-fade), an overlay's open window stays open. */
        memcpy(&px, src + (size_t)y * pitch + (size_t)(left ? 0 : w - 1) * 4u, 4);
        return px & 0xFF000000u;
    }
}

/* DXT5 (Xbox format 0x0F) blocks, in plain row order as Xbox compressed
 * textures keep them, into linear A8R8G8B8 (B, G, R, A in memory): the
 * top-left crop_w x crop_h texels of a texture tex_w texels wide. The track
 * loading pictures are 1024x512 DXT5 with their 640x480 of art top left. */
static void rgb565(uint16_t c, uint8_t *r, uint8_t *g, uint8_t *b)
{
    *r = (uint8_t)(((c >> 11) & 31u) * 255u / 31u);
    *g = (uint8_t)(((c >> 5) & 63u) * 255u / 63u);
    *b = (uint8_t)((c & 31u) * 255u / 31u);
}

void fz_backdrop_decode_dxt5(const uint8_t *src, uint32_t tex_w,
                             uint32_t crop_w, uint32_t crop_h, uint8_t *dst)
{
    uint32_t bx, by, blocks_w = tex_w / 4u, i;

    for (by = 0; by < (crop_h + 3u) / 4u; by++)
        for (bx = 0; bx < (crop_w + 3u) / 4u; bx++) {
            const uint8_t *blk = src + ((size_t)by * blocks_w + bx) * 16u;
            uint8_t a[8], c[4][3];
            uint64_t abits = 0;
            uint32_t cbits;
            uint16_t c0, c1;

            a[0] = blk[0];
            a[1] = blk[1];
            for (i = 0; i < 6; i++)
                abits |= (uint64_t)blk[2 + i] << (8u * i);
            if (a[0] > a[1])
                for (i = 1; i <= 6; i++)
                    a[1 + i] = (uint8_t)(((7u - i) * a[0] + i * a[1]) / 7u);
            else {
                for (i = 1; i <= 4; i++)
                    a[1 + i] = (uint8_t)(((5u - i) * a[0] + i * a[1]) / 5u);
                a[6] = 0;
                a[7] = 255;
            }
            c0 = (uint16_t)(blk[8] | blk[9] << 8);
            c1 = (uint16_t)(blk[10] | blk[11] << 8);
            cbits = (uint32_t)blk[12] | (uint32_t)blk[13] << 8 | (uint32_t)blk[14] << 16 |
                    (uint32_t)blk[15] << 24;
            rgb565(c0, &c[0][0], &c[0][1], &c[0][2]);
            rgb565(c1, &c[1][0], &c[1][1], &c[1][2]);
            for (i = 0; i < 3; i++) {
                c[2][i] = (uint8_t)((2u * c[0][i] + c[1][i]) / 3u);
                c[3][i] = (uint8_t)((c[0][i] + 2u * c[1][i]) / 3u);
            }
            for (i = 0; i < 16; i++) {
                uint32_t x = bx * 4u + (i & 3u), y = by * 4u + (i >> 2);
                const uint8_t *rgb = c[(cbits >> (2u * i)) & 3u];
                uint8_t *p;

                if (x >= crop_w || y >= crop_h)
                    continue;
                p = dst + ((size_t)y * crop_w + x) * 4u;
                p[0] = rgb[2];
                p[1] = rgb[1];
                p[2] = rgb[0];
                p[3] = a[(abits >> (3u * i)) & 7u];
            }
        }
}

/* Linear A8R8G8B8 (crop_w x crop_h at src, crop_w * 4 bytes a row) back into
 * DXT5 blocks of a texture tex_w texels wide, top left. A plain encoder:
 * each block's colour endpoints are the corners of its colour box and its
 * alpha endpoints its extremes, every texel the nearest of the eight or four
 * levels. The pictures were DXT5 to begin with, so this loses next to
 * nothing, and the texture keeps its own memory, size and format. */
static uint16_t to565(int r, int g, int b)
{
    return (uint16_t)(((r * 31 + 127) / 255) << 11 | ((g * 63 + 127) / 255) << 5 |
                      ((b * 31 + 127) / 255));
}

void fz_backdrop_encode_dxt5(const uint8_t *src, uint32_t crop_w, uint32_t crop_h,
                             uint8_t *dst, uint32_t tex_w)
{
    uint32_t bx, by, blocks_w = tex_w / 4u, i, j;

    for (by = 0; by < (crop_h + 3u) / 4u; by++)
        for (bx = 0; bx < (crop_w + 3u) / 4u; bx++) {
            uint8_t px[16][4], *blk = dst + ((size_t)by * blocks_w + bx) * 16u;
            int lo[4] = { 255, 255, 255, 255 }, hi[4] = { 0, 0, 0, 0 };
            uint8_t al[8], pal[4][3];
            uint64_t abits = 0;
            uint32_t cbits = 0;
            uint16_t c0, c1;

            for (i = 0; i < 16; i++) {
                uint32_t x = bx * 4u + (i & 3u), y = by * 4u + (i >> 2);

                if (x >= crop_w) x = crop_w - 1u;
                if (y >= crop_h) y = crop_h - 1u;
                memcpy(px[i], src + ((size_t)y * crop_w + x) * 4u, 4);
                for (j = 0; j < 4; j++) {
                    if (px[i][j] < lo[j]) lo[j] = px[i][j];
                    if (px[i][j] > hi[j]) hi[j] = px[i][j];
                }
            }
            /* Alpha: eight levels from the extremes. */
            al[0] = (uint8_t)hi[3];
            al[1] = (uint8_t)lo[3];
            for (i = 1; i <= 6; i++)
                al[1 + i] = (uint8_t)(((7u - i) * al[0] + i * al[1]) / 7u);
            for (i = 0; i < 16 && al[0] != al[1]; i++) {
                uint32_t best = 0, bd = 1u << 30;

                for (j = 0; j < 8; j++) {
                    uint32_t d = (uint32_t)abs((int)px[i][3] - (int)al[j]);
                    if (d < bd) { bd = d; best = j; }
                }
                abits |= (uint64_t)best << (3u * i);
            }
            /* Colour: the corners of the box, four levels (c0 > c1). Memory
             * order is B, G, R. */
            c0 = to565(hi[2], hi[1], hi[0]);
            c1 = to565(lo[2], lo[1], lo[0]);
            if (c0 < c1) { uint16_t t = c0; c0 = c1; c1 = t; }
            pal[0][0] = (uint8_t)(((c0 >> 11) & 31u) * 255u / 31u);
            pal[0][1] = (uint8_t)(((c0 >> 5) & 63u) * 255u / 63u);
            pal[0][2] = (uint8_t)((c0 & 31u) * 255u / 31u);
            pal[1][0] = (uint8_t)(((c1 >> 11) & 31u) * 255u / 31u);
            pal[1][1] = (uint8_t)(((c1 >> 5) & 63u) * 255u / 63u);
            pal[1][2] = (uint8_t)((c1 & 31u) * 255u / 31u);
            for (j = 0; j < 3; j++) {
                pal[2][j] = (uint8_t)((2u * pal[0][j] + pal[1][j]) / 3u);
                pal[3][j] = (uint8_t)((pal[0][j] + 2u * pal[1][j]) / 3u);
            }
            for (i = 0; i < 16 && c0 != c1; i++) {
                uint32_t best = 0, bd = 1u << 30;

                for (j = 0; j < 4; j++) {
                    int dr = (int)px[i][2] - pal[j][0], dg = (int)px[i][1] - pal[j][1];
                    int db = (int)px[i][0] - pal[j][2];
                    uint32_t d = (uint32_t)(dr * dr + dg * dg + db * db);
                    if (d < bd) { bd = d; best = j; }
                }
                cbits |= best << (2u * i);
            }
            blk[0] = al[0];
            blk[1] = al[1];
            for (i = 0; i < 6; i++)
                blk[2 + i] = (uint8_t)(abits >> (8u * i));
            blk[8] = (uint8_t)c0;
            blk[9] = (uint8_t)(c0 >> 8);
            blk[10] = (uint8_t)c1;
            blk[11] = (uint8_t)(c1 >> 8);
            for (i = 0; i < 4; i++)
                blk[12 + i] = (uint8_t)(cbits >> (8u * i));
        }
}

void fz_backdrop_squeeze(const uint8_t *src, uint32_t w, uint32_t h, uint8_t *dst, uint32_t nw)
{
    uint32_t y, x, c;

    /* Each new texel the average of the old ones it covers (a box filter):
     * a squeeze by 3/4 never skips a stripe. */
    for (y = 0; y < h; y++)
        for (x = 0; x < nw; x++) {
            double a = (double)x * w / nw, b = (double)(x + 1) * w / nw, acc[4] = { 0, 0, 0, 0 };
            uint32_t i0 = (uint32_t)a, i;

            for (i = i0; i < w && (double)i < b; i++) {
                double lo = (double)i > a ? (double)i : a, hi = (double)(i + 1) < b ? (double)(i + 1) : b;
                const uint8_t *p = src + ((size_t)y * w + i) * 4u;

                for (c = 0; c < 4; c++)
                    acc[c] += p[c] * (hi - lo);
            }
            for (c = 0; c < 4; c++)
                dst[((size_t)y * nw + x) * 4u + c] = (uint8_t)(acc[c] / (b - a) + 0.5);
        }
}

void fz_backdrop_widen(const uint8_t *src, uint32_t w, uint32_t h, uint32_t src_pitch,
                       uint8_t *dst, uint32_t dst_pitch,
                       uint32_t add_left, FzFill left_mode,
                       uint32_t add_right, FzFill right_mode)
{
    uint32_t y, x, nw = w + add_left + add_right;

    g_band_src = NULL;

    for (y = 0; y < h; y++) {
        uint8_t *row = dst + (size_t)y * dst_pitch;

        for (x = 0; x < nw; x++) {
            int xs = (int)x - (int)add_left;
            uint32_t px;

            if (xs >= 0 && xs < (int)w)
                memcpy(&px, src + (size_t)y * src_pitch + (size_t)xs * 4u, 4);
            else
                px = fill(xs < 0 ? left_mode : right_mode, src, w, h, src_pitch, xs, y);
            memcpy(row + (size_t)x * 4u, &px, 4);
        }
        memset(row + (size_t)nw * 4u, 0, dst_pitch - (size_t)nw * 4u);
    }
}

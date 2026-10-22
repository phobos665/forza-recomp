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
#include <string.h>

#include "fz_backdrop.h"

#define FZ_STRIPE_SLOPE  1.2
#define FZ_STRIPE_REPEAT 18

static uint32_t sample_stripes(const uint8_t *src, uint32_t w, uint32_t h, uint32_t pitch,
                               int x_new, uint32_t y, int left)
{
    /* x_new < 0 left of the art, >= w right of it. Walk along the stripe
     * (rising to the right) to a column two texels inside the art. */
    int edge = left ? 2 : (int)w - 3;
    double t = (double)(edge - x_new);          /* > 0 on the left, < 0 on the right */
    double ys = (double)y - FZ_STRIPE_SLOPE * t;
    uint32_t px;

    while (ys < 0.0)
        ys += FZ_STRIPE_REPEAT;
    while (ys >= (double)h)
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

void fz_backdrop_widen(const uint8_t *src, uint32_t w, uint32_t h, uint32_t src_pitch,
                       uint8_t *dst, uint32_t dst_pitch,
                       uint32_t add_left, FzFill left_mode,
                       uint32_t add_right, FzFill right_mode)
{
    uint32_t y, x, nw = w + add_left + add_right;

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

/* fz_backdrop.h -- Forza Motorsport's menu backgrounds, widened for 16:9
 * (fz_backdrop.c). */
#ifndef FZ_BACKDROP_H
#define FZ_BACKDROP_H

#include <stdint.h>

typedef enum { FZ_FILL_CLEAR = 0, FZ_FILL_CLAMP, FZ_FILL_STRIPES } FzFill;

/* A linear 32-bit picture w x h at src (src_pitch bytes a row) into dst,
 * add_left + w + add_right texels wide (dst_pitch bytes a row), the new
 * columns made as each side's mode says. */
void fz_backdrop_widen(const uint8_t *src, uint32_t w, uint32_t h, uint32_t src_pitch,
                       uint8_t *dst, uint32_t dst_pitch,
                       uint32_t add_left, FzFill left_mode,
                       uint32_t add_right, FzFill right_mode);

#endif

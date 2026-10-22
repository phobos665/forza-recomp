/* fz_backdrop.h -- Forza Motorsport's menu backgrounds, widened for 16:9
 * (fz_backdrop.c). */
#ifndef FZ_BACKDROP_H
#define FZ_BACKDROP_H

#include <stdint.h>

typedef enum { FZ_FILL_CLEAR = 0, FZ_FILL_CLAMP, FZ_FILL_STRIPES } FzFill;

/* The top-left crop_w x crop_h texels of a DXT5 texture tex_w wide, into
 * linear 32-bit texels at dst (crop_w * 4 bytes a row). */
void fz_backdrop_decode_dxt5(const uint8_t *src, uint32_t tex_w,
                             uint32_t crop_w, uint32_t crop_h, uint8_t *dst);

/* Linear 32-bit texels (crop_w x crop_h at src) into the top left of a
 * DXT5 texture tex_w texels wide, in place. */
void fz_backdrop_encode_dxt5(const uint8_t *src, uint32_t crop_w, uint32_t crop_h,
                             uint8_t *dst, uint32_t tex_w);

/* w x h linear 32-bit texels squeezed (or stretched) to nw x h. */
void fz_backdrop_squeeze(const uint8_t *src, uint32_t w, uint32_t h, uint8_t *dst, uint32_t nw);

/* A linear 32-bit picture w x h at src (src_pitch bytes a row) into dst,
 * add_left + w + add_right texels wide (dst_pitch bytes a row), the new
 * columns made as each side's mode says. */
void fz_backdrop_widen(const uint8_t *src, uint32_t w, uint32_t h, uint32_t src_pitch,
                       uint8_t *dst, uint32_t dst_pitch,
                       uint32_t add_left, FzFill left_mode,
                       uint32_t add_right, FzFill right_mode);

#endif

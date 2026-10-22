/* fz_layout.h -- Forza Motorsport's menus, laid out for 16:9 (fz_layout.c). */
#ifndef FZ_LAYOUT_H
#define FZ_LAYOUT_H

#include <stddef.h>
#include <stdint.h>

#include "fz_backdrop.h"

/* A layout file is about to be read: `path` is the file's own path, as the
 * game opened it. Files outside D:\MEDIA\UI\ are left alone. Nests. */
void fz_layout_begin_file(const char *path);
void fz_layout_end_file(void);

/* Whether the file being read is a layout this changes. */
int  fz_layout_active(void);

/* One "Key value" line of it, rewritten in place (`cap` bytes) when a rule
 * applies; 1 if it did. `extra` gets more lines to hand the game after this
 * one, newline-separated, or "" -- how a value already read is corrected
 * once a later line shows it needed to be. */
int  fz_layout_line(char *line, size_t cap, char *extra, size_t extra_cap);

/* Whether the background at `path` (w x h texels) is widened for 16:9, and
 * if so how: the new columns on each side and how each side is made. */
int  fz_layout_texture_recipe(const char *path, uint32_t w, uint32_t h,
                              uint32_t *add_left, FzFill *left,
                              uint32_t *add_right, FzFill *right);

#endif

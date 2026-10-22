/*
 * fz_layout.c -- Forza Motorsport's menus, laid out for 16:9.
 *
 * Every screen of the front end is a layout file, D:\MEDIA\UI\NTSC\<name>.ini
 * (from cabs\ui_NTSC.ca2), of lines "Key value": "Menu\Left 55",
 * "Texture\TextureRect\Width 720". Positions are in a 640x480 space and
 * relative to the parent control; a screen's own root holds the rest. The
 * game reads each file line by line (sub_00272980) and hands every line to
 * the property tree (sub_00271870); recomp_manual.c hooks both and passes
 * the lines through here first, so a rule can change a value before the game
 * ever sees it. No layout data is shipped: the rules say how to move what the
 * player's own disc holds.
 *
 * With the UI squish held at 4/3 (recomp_manual.c, "Widescreen") x from
 * -106.67 to 746.67 reaches the edges of the 16:9 picture, and 0..640 is the
 * middle. So "pin to the left edge" is Left - 106.67, "to the right" is
 * Left + 106.67, and "span" is Left - 106.67 and Width + 213.33.
 *
 * Rules (layout/widescreen.txt, compiled in; FZ_LAYOUT=<file> reads another
 * at run time, for working on a layout without rebuilding), one a line:
 *
 *   <screen> <key> <op> <number>
 *
 * screen  the file's name without .ini, any case, or * for every screen
 * key     the key exactly as the file has it (any case)
 * op      =  set the value
 *         +  add to it
 *         -  subtract from it
 *         left  / right   (no number) pin to that edge: - or + 106.67
 *         span            (no number) on a Width key: + 213.33
 *
 * The backgrounds themselves are widened to 4/3 of their width when the game
 * loads them (fz_backdrop.c), each by a recipe:
 *
 *   texture <name> <left fill> <right fill> [<percent of the new columns on
 *           the left, 50 by default> [fit]]
 *
 * "fit" squeezes the widened picture back into the texture's old width, for
 * a picture the game draws from a fixed part of its texture whatever the
 * layout says (the track loading pictures): drawn 4/3 wider across the
 * spanned control, it comes out in proportion.
 *
 * name is the file's name without .xpr, any case, "prefix*" for a family
 * of them, or "default" for a background (a bkgd* file) with no recipe of
 * its own. Fills are stripes, clamp and clear (fz_backdrop.c).
 *
 * And two switches. "<screen> keep" leaves a screen as the game lays it out
 * (its rules still apply, nothing else does): the race's own windows,
 * whose HUD is the game's own 16:9 one and places itself from its window.
 *
 * "* span-full-width". Every picture directly under a
 * screen's root that covers all 640 pixels (a control with a texture
 * rectangle starting at texel 0) spans the 16:9 picture, and its texture
 * rectangle grows by 4/3 from its left edge, so it shows the widened texture
 * whole. A picture showing its whole texture (rectangle width 0) just spans.
 * A gradient bar directly under a spanned root (corner colours, no texture)
 * that touches an edge of the 4:3 screen reaches that edge of the 16:9 one.
 * Every screen's root that covers the screen spans it too, and every control
 * directly under it moves back by the same 106.67, so the content stays
 * where it was and only what the root draws widens: its BackgroundTexture,
 * or the background its code gives it. Containers are left alone --
 * spanning one would move what is in it.
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fz_layout.h"

#define FZ_EDGE      106.666667     /* (4/3 * 640 - 640) / 2 */
#define FZ_WIDEN     (4.0 / 3.0)    /* how much wider a background becomes */
#define FZ_MAX_RULES 1024
#define FZ_MAX_TEX   128

typedef enum { OP_SET, OP_ADD, OP_SUB, OP_LEFT, OP_RIGHT, OP_SPAN } FzOp;

typedef struct {
    char  screen[64];   /* lower case; "*" for all */
    char  key[160];     /* lower case */
    FzOp  op;
    double value;
} FzRule;

typedef struct {
    char   name[64];    /* lower case, no extension */
    FzFill left, right;
    int    left_percent;
    int    fit;         /* squeeze the widened picture back into its old width */
} FzTexRule;

static FzRule    g_rules[FZ_MAX_RULES];
static int       g_nrules;
static FzTexRule g_tex[FZ_MAX_TEX];
static int       g_ntex;
static int       g_span_full_width;
static char      g_keep[32][64];    /* screens left as the game lays them out */
static int       g_nkeep;
static int       g_loaded;

extern const unsigned char fz_layout_rules_text[];  /* generated from layout/widescreen.txt */

static void lower(char *s)
{
    for (; *s; s++)
        *s = (char)tolower((unsigned char)*s);
}

static int parse_fill(const char *s, FzFill *out)
{
    if (!strcmp(s, "stripes")) { *out = FZ_FILL_STRIPES; return 1; }
    if (!strcmp(s, "clamp"))   { *out = FZ_FILL_CLAMP;   return 1; }
    if (!strcmp(s, "clear"))   { *out = FZ_FILL_CLEAR;   return 1; }
    return 0;
}

static void parse_rules(const char *text)
{
    const char *p = text;

    while (*p) {
        char line[512], screen[64], key[160], op[16], extra[16];
        double value = 0.0;
        size_t n = strcspn(p, "\r\n");
        int got;

        if (n >= sizeof line)
            n = sizeof line - 1;
        memcpy(line, p, n);
        line[n] = 0;
        p += n;
        while (*p == '\r' || *p == '\n')
            p++;
        if (line[0] == '#' || line[0] == 0)
            continue;
        if (!strncmp(line, "texture ", 8) && g_ntex < FZ_MAX_TEX) {
            FzTexRule *t = &g_tex[g_ntex];
            char lf[16], rf[16], opt[16] = "";
            int pct = 50;

            got = sscanf(line + 8, "%63s %15s %15s %d %15s", t->name, lf, rf, &pct, opt);
            if (got >= 3 && parse_fill(lf, &t->left) && parse_fill(rf, &t->right)) {
                lower(t->name);
                t->left_percent = pct < 0 ? 0 : pct > 100 ? 100 : pct;
                t->fit = !strcmp(opt, "fit");
                g_ntex++;
            } else {
                fprintf(stderr, "[FZLAYOUT] not a texture recipe: %s\n", line);
            }
            continue;
        }
        got = sscanf(line, "%63s %159s %15s %lf %15s", screen, key, op, &value, extra);
        if (got == 2 && !strcmp(screen, "*") && !strcmp(key, "span-full-width")) {
            g_span_full_width = 1;
            continue;
        }
        if (got == 2 && !strcmp(key, "keep") && strcmp(screen, "*")) {
            if (g_nkeep < (int)(sizeof g_keep / sizeof g_keep[0])) {
                snprintf(g_keep[g_nkeep], sizeof g_keep[0], "%s", screen);
                lower(g_keep[g_nkeep++]);
            }
            continue;
        }
        if (got < 3 || g_nrules == FZ_MAX_RULES) {
            fprintf(stderr, "[FZLAYOUT] not a rule: %s\n", line);
            continue;
        }
        {
            FzRule *r = &g_rules[g_nrules];
            int ok = 1;

            snprintf(r->screen, sizeof r->screen, "%s", screen);
            snprintf(r->key, sizeof r->key, "%s", key);
            lower(r->screen);
            lower(r->key);
            r->value = value;
            if (!strcmp(op, "=") && got >= 4)       r->op = OP_SET;
            else if (!strcmp(op, "+") && got >= 4)  r->op = OP_ADD;
            else if (!strcmp(op, "-") && got >= 4)  r->op = OP_SUB;
            else if (!strcmp(op, "left"))           r->op = OP_LEFT;
            else if (!strcmp(op, "right"))          r->op = OP_RIGHT;
            else if (!strcmp(op, "span"))           r->op = OP_SPAN;
            else ok = 0;
            if (ok)
                g_nrules++;
            else
                fprintf(stderr, "[FZLAYOUT] not a rule: %s\n", line);
        }
    }
}

static void load_rules(void)
{
    const char *path = getenv("FZ_LAYOUT");

    if (g_loaded)
        return;
    g_loaded = 1;
    if (path && *path) {
        FILE *f = fopen(path, "rb");
        if (f) {
            char *text;
            long len;

            fseek(f, 0, SEEK_END);
            len = ftell(f);
            fseek(f, 0, SEEK_SET);
            text = (char *)malloc((size_t)len + 1);
            if (text && fread(text, 1, (size_t)len, f) == (size_t)len) {
                text[len] = 0;
                parse_rules(text);
                fprintf(stderr, "[FZLAYOUT] %d rules, %d texture recipes from %s\n",
                        g_nrules, g_ntex, path);
            }
            free(text);
            fclose(f);
            return;
        }
        fprintf(stderr, "[FZLAYOUT] cannot read %s; using the built-in rules\n", path);
    }
    parse_rules((const char *)fz_layout_rules_text);
    fprintf(stderr, "[FZLAYOUT] %d rules, %d texture recipes%s\n", g_nrules, g_ntex,
            g_span_full_width ? ", full-width pictures spanned" : "");
}

/* ── Backgrounds ─────────────────────────────────────────────────── */

int fz_layout_texture_recipe(const char *path, uint32_t w, uint32_t h,
                             uint32_t *add_left, FzFill *left,
                             uint32_t *add_right, FzFill *right, int *fit)
{
    const char *base;
    char name[64];
    const FzTexRule *t = NULL, *dflt = NULL;
    uint32_t extra;
    size_t n;
    int i;

    load_rules();
    if (!g_span_full_width || !path || (w != 720 && w != 640) || h < 300)
        return 0;
    base = strrchr(path, '\\');
    base = base ? base + 1 : path;
    n = strcspn(base, ".");
    if (n >= sizeof name)
        n = sizeof name - 1;
    memcpy(name, base, n);
    name[n] = 0;
    lower(name);
    for (i = 0; i < g_ntex && !t; i++) {
        size_t k = strlen(g_tex[i].name);

        /* "name", or "prefix*" for a family. */
        if (!strcmp(g_tex[i].name, name) ||
            (k > 1 && g_tex[i].name[k - 1] == '*' && !strncmp(g_tex[i].name, name, k - 1)))
            t = &g_tex[i];
        else if (!strcmp(g_tex[i].name, "default"))
            dflt = &g_tex[i];
    }
    if (!t) {
        /* Only backgrounds: a texture shown at its own size somewhere
         * would be shifted by its new columns. */
        if (strncmp(name, "bkgd", 4) || !dflt)
            return 0;
        t = dflt;
    }
    extra = (uint32_t)((double)w * FZ_WIDEN + 0.5) - w;     /* 240, or 213 */
    *add_left = (extra * (uint32_t)t->left_percent + 50u) / 100u;
    *add_right = extra - *add_left;
    *left = t->left;
    *right = t->right;
    *fit = t->fit;
    return 1;
}

/* ── The file being read ─────────────────────────────────────────── */

#define FZ_DEPTH    8
#define FZ_CHILDREN 160

/* Per file being read. */
typedef struct {
    char   screen[64];
    /* The top-level control in hand, until its texture rectangle shows
     * whether it is a picture. Its Left and Width have been read by then,
     * so a span sends them again (fz_layout_line's extra lines). */
    char   ctl[96];          /* lower case: "texture", or "\\" for an unnamed child */
    char   spell[96];        /* the key's own prefix: "Texture\\" or "\\" */
    double left, width, tr_left;
    int    have_left, have_width;
    /* The root: whether it covers the screen, and whether it was spanned
     * (then every top-level Left is moved back by FZ_EDGE). */
    double root_left, root_width;
    int    root_have_left, root_have_width, root_spanned;
    /* Top-level Lefts read so far, as sent: a root spanned late moves
     * them back by sending them again. */
    char   child_spell[FZ_CHILDREN][96];
    double child_left[FZ_CHILDREN];
    int    nchildren;
    int    keep;             /* "<screen> keep": only its own rules */
} FzFile;

static FzFile g_file[FZ_DEPTH];
static int    g_depth;

void fz_layout_begin_file(const char *path)
{
    const char *base, *ui;
    FzFile *f;
    size_t n;
    int i;

    if (g_depth >= FZ_DEPTH) {
        g_depth++;
        return;
    }
    f = &g_file[g_depth++];
    memset(f, 0, sizeof *f);
    ui = path ? strstr(path, "\\UI\\") : NULL;
    if (!ui && path)
        ui = strstr(path, "\\ui\\");
    if (!ui)
        return;
    base = strrchr(path, '\\') + 1;
    n = strcspn(base, ".");
    if (n >= sizeof f->screen)
        n = sizeof f->screen - 1;
    memcpy(f->screen, base, n);
    f->screen[n] = 0;
    lower(f->screen);
    load_rules();
    for (i = 0; i < g_nkeep; i++)
        if (!strcmp(g_keep[i], f->screen))
            f->keep = 1;
    if (getenv("FZ_LAYOUT_LOG"))
        fprintf(stderr, "[FZLAYOUT] reading %s\n", f->screen);
}

void fz_layout_end_file(void)
{
    if (g_depth > 0)
        g_depth--;
}

int fz_layout_active(void)
{
    return g_depth > 0 && g_depth <= FZ_DEPTH && g_file[g_depth - 1].screen[0];
}

/* ── One line ────────────────────────────────────────────────────── */

static const FzRule *find_rule(const char *screen, const char *key)
{
    int i;

    for (i = 0; i < g_nrules; i++)
        if ((g_rules[i].screen[0] == '*' || !strcmp(g_rules[i].screen, screen)) &&
            !strcmp(g_rules[i].key, key))
            return &g_rules[i];
    return NULL;
}

static double apply(const FzRule *r, double v)
{
    switch (r->op) {
    case OP_SET:   return r->value;
    case OP_ADD:   return v + r->value;
    case OP_SUB:   return v - r->value;
    case OP_LEFT:  return v - FZ_EDGE;
    case OP_RIGHT: return v + FZ_EDGE;
    case OP_SPAN:  return v + 2.0 * FZ_EDGE;
    }
    return v;
}

/* "a\b\Left" -> control "a\b", field "left". Both lower case; a key with
 * no backslash is the root's own: control "". */
static const char *split_key(const char *key, char *ctl, size_t cap)
{
    const char *slash = strrchr(key, '\\');
    size_t n = slash ? (size_t)(slash - key) : 0;

    if (n >= cap)
        n = cap - 1;
    memcpy(ctl, key, n);
    ctl[n] = 0;
    return slash ? slash + 1 : key;
}

static int top_level(const char *ctl)
{
    /* "X" directly under the root, or "\\": an unnamed child, written "\Left". */
    return ctl[0] && (!strchr(ctl, '\\') || !strcmp(ctl, "\\"));
}

static void add_extra(char *extra, size_t cap, const char *fmt_key, double v)
{
    size_t used = strlen(extra);

    snprintf(extra + used, cap - used, "%s%s %.4f", used ? "\n" : "", fmt_key, v);
}

/* A top-level Left as it will be sent, remembered for a late root span. */
static void note_child(FzFile *f, const char *spell, double sent)
{
    int i;

    for (i = 0; i < f->nchildren; i++)
        if (!strcmp(f->child_spell[i], spell)) {
            f->child_left[i] = sent;
            return;
        }
    if (f->nchildren < FZ_CHILDREN) {
        snprintf(f->child_spell[f->nchildren], sizeof f->child_spell[0], "%s", spell);
        f->child_left[f->nchildren++] = sent;
    }
}

/* A screen's root that covers the screen spans the picture -- what it
 * draws itself (its BackgroundTexture, or one its code gives it) widens --
 * and every top-level control moves back so it stays where it was. */
static void span_root(FzFile *f, char *extra, size_t cap)
{
    int i;
    char k[112];

    if (f->root_spanned || !f->root_have_left || !f->root_have_width ||
        f->root_left > 0.5 || f->root_left + f->root_width < 639.5)
        return;
    f->root_spanned = 1;
    add_extra(extra, cap, "Left", f->root_left - FZ_EDGE);
    add_extra(extra, cap, "Width", f->root_width + 2.0 * FZ_EDGE);
    for (i = 0; i < f->nchildren; i++) {
        snprintf(k, sizeof k, "%sLeft", f->child_spell[i]);
        f->child_left[i] += FZ_EDGE;
        add_extra(extra, cap, k, f->child_left[i]);
    }
}

int fz_layout_line(char *line, size_t cap, char *extra, size_t extra_cap)
{
    char key[160], lkey[160], ctl[96], *sp;
    const char *field;
    FzFile *f;
    const FzRule *r;
    double v, nv;
    int changed = 0;

    extra[0] = 0;
    if (!fz_layout_active())
        return 0;
    f = &g_file[g_depth - 1];
    sp = strchr(line, ' ');
    if (!sp || (size_t)(sp - line) >= sizeof key)
        return 0;
    memcpy(key, line, (size_t)(sp - line));
    key[sp - line] = 0;
    snprintf(lkey, sizeof lkey, "%s", key);
    lower(lkey);

    /* Lists the game exempts from its own squish (DoWidescreenSquish false,
     * 122 of them) draw without it: off to one side of everything else in
     * the picture. With the squish held on everywhere they take it too. */
    {
        size_t kl = strlen(lkey);

        if (!f->keep && kl >= 18 && !strcmp(lkey + kl - 18, "dowidescreensquish")) {
            if (strncmp(sp + 1, "false", 5) || (size_t)(sp - line) + 6 >= cap)
                return 0;
            memcpy(sp + 1, "true", 5);
            if (getenv("FZ_LAYOUT_LOG"))
                fprintf(stderr, "[FZLAYOUT] %s: %s false -> true\n", f->screen, key);
            return 1;
        }
    }
    if (sscanf(sp + 1, "%lf", &v) != 1)
        return 0;
    nv = v;
    field = split_key(lkey, ctl, sizeof ctl);
    if (!ctl[0] && lkey[0] == '\\')
        snprintf(ctl, sizeof ctl, "\\");

    if (!ctl[0] && !strcmp(field, "left")) {
        f->root_left = v;
        f->root_have_left = 1;
    } else if (!ctl[0] && !strcmp(field, "width")) {
        f->root_width = v;
        f->root_have_width = 1;
    }

    r = find_rule(f->screen, lkey);
    if (r) {
        nv = apply(r, v);
        changed = 1;
    } else if (g_span_full_width && !f->keep) {
        if (!ctl[0] && !strcmp(field, "width")) {
            /* The root: its Left went by first. */
            span_root(f, extra, extra_cap);
            if (f->root_spanned) {
                nv = v + 2.0 * FZ_EDGE;
                changed = 1;
                extra[0] = 0;
                add_extra(extra, extra_cap, "Left", f->root_left - FZ_EDGE);
            }
        } else if (top_level(ctl) && !strcmp(field, "left")) {
            memset(f->ctl, 0, sizeof f->ctl);
            snprintf(f->ctl, sizeof f->ctl, "%s", ctl);
            snprintf(f->spell, sizeof f->spell, "%.*s", (int)(strlen(key) - strlen("Left")), key);
            f->left = v;
            f->have_left = 1;
            f->have_width = 0;
            f->tr_left = 0.0;
            if (f->root_spanned) {
                nv = v + FZ_EDGE;
                changed = 1;
            }
            note_child(f, f->spell, nv);
        } else if (f->have_left && !strcmp(ctl, f->ctl) && !strcmp(field, "width")) {
            f->width = v;
            f->have_width = 1;
        } else if (f->have_width && f->root_spanned && !strcmp(field, "alpha") &&
                   strlen(ctl) == strlen(f->ctl) + 8 && !strncmp(ctl, f->ctl, strlen(f->ctl)) &&
                   !strcmp(ctl + strlen(f->ctl), "\\topleft")) {
            /* A gradient bar (a rectangle with corner colours, no texture):
             * one that touches an edge of the 4:3 screen reaches the same
             * edge of the 16:9 one, as it did on the console. Nothing is
             * stretched; it is flat colour. */
            int at_left = f->left <= 0.5, at_right = f->left + f->width >= 636.5;

            if (at_left || at_right) {
                double l = at_left ? -FZ_EDGE : f->left;
                double r = at_right ? (f->left + f->width > 640.0 ? f->left + f->width : 640.0) + FZ_EDGE
                                    : f->left + f->width;
                char k[112];

                snprintf(k, sizeof k, "%sLeft", f->spell);
                add_extra(extra, extra_cap, k, l + FZ_EDGE);
                snprintf(k, sizeof k, "%sWidth", f->spell);
                add_extra(extra, extra_cap, k, r - l);
                note_child(f, f->spell, l + FZ_EDGE);
            }
            f->have_left = f->have_width = 0;
        } else if (f->have_width) {
            char want[128];

            snprintf(want, sizeof want, "%s\\texturerect", !strcmp(f->ctl, "\\") ? "" : f->ctl);
            if (!strcmp(ctl, want) && !strcmp(field, "left")) {
                f->tr_left = v;
            } else if (!strcmp(ctl, want) && !strcmp(field, "width")) {
                if (f->left <= 0.5 && f->left + f->width >= 639.5 && f->tr_left == 0.0 &&
                    f->root_have_width && f->root_left <= 0.5 &&
                    f->root_left + f->root_width >= 639.5) {
                    /* A picture across the whole 4:3 screen: span it, and
                     * let its texture rectangle take in the widened
                     * texture whole (or the whole texture already: 0). */
                    double left = f->left - FZ_EDGE + (f->root_spanned ? FZ_EDGE : 0.0);
                    char k[112];

                    if (v > 0.0) {
                        nv = v * FZ_WIDEN;
                        changed = 1;
                    }
                    snprintf(k, sizeof k, "%sLeft", f->spell);
                    add_extra(extra, extra_cap, k, left);
                    snprintf(k, sizeof k, "%sWidth", f->spell);
                    add_extra(extra, extra_cap, k, f->width + 2.0 * FZ_EDGE);
                    note_child(f, f->spell, left);
                }
                f->have_left = f->have_width = 0;
            }
        }
    }
    if (!changed && !extra[0])
        return 0;
    if (changed && (size_t)snprintf(line, cap, "%s %.4f", key, nv) >= cap)
        return 0;
    if (getenv("FZ_LAYOUT_LOG"))
        fprintf(stderr, "[FZLAYOUT] %s: %s %g -> %.4f%s%s\n", f->screen, key, v, nv,
                extra[0] ? "; then " : "", extra);
    return 1;
}

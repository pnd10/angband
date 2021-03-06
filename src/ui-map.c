/**
 * \file ui-map.c
 * \brief Writing level map info to the screen
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "cave.h"
#include "grafmode.h"
#include "init.h"
#include "mon-util.h"
#include "monster.h"
#include "obj-util.h"
#include "player-timed.h"
#include "trap.h"
#include "ui-input.h"
#include "ui-map.h"
#include "ui-object.h"
#include "ui-output.h"
#include "ui-prefs.h"
#include "ui-term.h"


/**
 * Hack -- Hallucinatory monster
 */
static void hallucinatory_monster(int *a, wchar_t *c)
{
	while (1) {
		/* Select a random monster */
		monster_race *r_ptr = &r_info[randint0(z_info->r_max)];
		
		/* Skip non-entries */
		if (!r_ptr->name) continue;
		
		/* Retrieve attr/char */
		*a = monster_x_attr[r_ptr->ridx];
		*c = monster_x_char[r_ptr->ridx];
		return;
	}
}


/**
 * Hack -- Hallucinatory object
 */
static void hallucinatory_object(int *a, wchar_t *c)
{
	
	while (1) {
		/* Select a random object */
		object_kind *k_ptr = &k_info[randint0(z_info->k_max - 1) + 1];

		/* Skip non-entries */
		if (!k_ptr->name) continue;
		
		/* Retrieve attr/char (HACK - without flavors) */
		*a = kind_x_attr[k_ptr->kidx];
		*c = kind_x_char[k_ptr->kidx];
		
		/* HACK - Skip empty entries */
		if (*a == 0 || *c == 0) continue;

		return;
	}
}


/**
 * Get the graphics of a listed trap.
 *
 * We should probably have better handling of stacked traps, but that can
 * wait until we do, in fact, have stacked traps under normal conditions.
 */
static void get_trap_graphics(struct chunk *c, grid_data *g, int *a, wchar_t *w)
{
    /* Trap is visible */
    if (trf_has(g->trap->flags, TRF_VISIBLE)) {
		/* Get the graphics */
		*a = trap_x_attr[g->lighting][g->trap->kind->tidx];
		*w = trap_x_char[g->lighting][g->trap->kind->tidx];
    }
}

/**
 * Apply text lighting effects
 */
static void grid_get_attr(grid_data *g, int *a)
{
    feature_type *f_ptr = &f_info[g->f_idx];

	/* Save the high-bit, since it's used for attr inversion in GCU */
	int a0 = *a & 0x80;

	/* Remove the high bit so we can add it back again at the end */
	*a = (*a & 0x7F);

	/* Never play with fg colours for treasure */
	if (!feat_is_treasure(g->f_idx)) {

		/* Tint trap detection borders */
		if (g->trapborder)
			*a = (g->in_view ? COLOUR_L_GREEN : COLOUR_GREEN);

		/* Only apply lighting effects when the attr is white --
		 * this is to stop e.g. doors going grey when out of LOS */
		if (*a == COLOUR_WHITE) {
			/* If it's a floor tile then we'll tint based on lighting. */
			if (tf_has(f_ptr->flags, TF_TORCH))
				switch (g->lighting) {
					case LIGHTING_TORCH: *a = COLOUR_YELLOW; break;
					case LIGHTING_LIT: *a = COLOUR_L_DARK; break;
					case LIGHTING_DARK: *a = COLOUR_L_DARK; break;
					default: break;
				}

			/* If it's another kind of tile, only tint when unlit. */
			else if (g->lighting == LIGHTING_DARK ||
					 g->lighting == LIGHTING_LIT)
				*a = COLOUR_L_DARK;
		} else if (feat_is_magma(g->f_idx) || feat_is_quartz(g->f_idx)) {
			if (!g->in_view) {
				*a = COLOUR_L_DARK;
			}
		}
	}

	/* Hybrid or block walls -- for GCU, then for everyone else */
	if (a0) {
		*a = a0 | *a;
	} else if (use_graphics == GRAPHICS_NONE && feat_is_wall(g->f_idx)) {
		if (OPT(hybrid_walls))
			*a = *a + (MAX_COLORS * BG_DARK);
		else if (OPT(solid_walls))
			*a = *a + (MAX_COLORS * BG_SAME);
	}
}

/**
 * This function takes a pointer to a grid info struct describing the 
 * contents of a grid location (as obtained through the function map_info)
 * and fills in the character and attr pairs for display.
 *
 * ap and cp are filled with the attr/char pair for the monster, object or 
 * floor tile that is at the "top" of the grid (monsters covering objects, 
 * which cover floor, assuming all are present).
 *
 * tap and tcp are filled with the attr/char pair for the floor, regardless
 * of what is on it.  This can be used by graphical displays with
 * transparency to place an object onto a floor tile, is desired.
 *
 * Any lighting effects are also applied to these pairs, clear monsters allow
 * the underlying colour or feature to show through (ATTR_CLEAR and
 * CHAR_CLEAR), multi-hued colour-changing (ATTR_MULTI) is applied, and so on.
 * Technically, the flag "CHAR_MULTI" is supposed to indicate that a monster 
 * looks strange when examined, but this flag is currently ignored.
 *
 * NOTES:
 * This is called pretty frequently, whenever a grid on the map display
 * needs updating, so don't overcomplicate it.
 *
 * The "zero" entry in the feature/object/monster arrays are
 * used to provide "special" attr/char codes, with "monster zero" being
 * used for the player attr/char, "object zero" being used for the "pile"
 * attr/char, and "feature zero" being used for the "darkness" attr/char.
 *
 * TODO:
 * The transformations for tile colors, or brightness for the 16x16
 * tiles should be handled differently.  One possibility would be to
 * extend feature_type with attr/char definitions for the different states.
 * This will probably be done outside of the current text->graphics mappings
 * though.
 */
void grid_data_as_text(grid_data *g, int *ap, wchar_t *cp, int *tap, wchar_t *tcp)
{
	feature_type *f_ptr = &f_info[g->f_idx];

	int a = feat_x_attr[g->lighting][f_ptr->fidx];
	wchar_t c = feat_x_char[g->lighting][f_ptr->fidx];

	/* Check for trap detection boundaries */
	if (use_graphics == GRAPHICS_NONE)
		grid_get_attr(g, &a);
	else if (g->trapborder && tf_has(f_ptr->flags, TF_FLOOR)
			 && (g->m_idx || g->first_kind)) {
		/* if there is an object or monster here, and this is a plain floor
		 * display the border here rather than an overlay below */
		a = feat_x_attr[g->lighting][FEAT_DTRAP_FLOOR];
		c = feat_x_char[g->lighting][FEAT_DTRAP_FLOOR];
	}

	/* Save the terrain info for the transparency effects */
	(*tap) = a;
	(*tcp) = c;

	/* There is a trap in this grid, and we are not hallucinating */
	if (g->trap && (!g->hallucinate))
	    /* Change graphics to indicate a trap (if visible) */
	    get_trap_graphics(cave, g, &a, &c);

	/* If there's an object, deal with that. */
	if (g->unseen_money) {
	
		/* $$$ gets an orange star*/
		a = object_kind_attr(&k_info[7]);
		c = object_kind_char(&k_info[7]);
		
	} else if (g->unseen_object) {	
	
		/* Everything else gets a red star */    
		a = object_kind_attr(&k_info[6]);
		c = object_kind_char(&k_info[6]);
		
	} else if (g->first_kind) {
		if (g->hallucinate) {
			/* Just pick a random object to display. */
			hallucinatory_object(&a, &c);
		} else if (g->multiple_objects) {
			/* Get the "pile" feature instead */
			a = object_kind_attr(&k_info[0]);
			c = object_kind_char(&k_info[0]);
		} else {
			/* Normal attr and char */
			a = object_kind_attr(g->first_kind);
			c = object_kind_char(g->first_kind);
		}
	}

	/* Handle monsters, the player and trap borders */
	if (g->m_idx > 0) {
		if (g->hallucinate) {
			/* Just pick a random monster to display. */
			hallucinatory_monster(&a, &c);
		} else if (!is_mimicking(cave_monster(cave, g->m_idx)))	{
			monster_type *m_ptr = cave_monster(cave, g->m_idx);

			byte da;
			wchar_t dc;

			/* Desired attr & char */
			da = monster_x_attr[m_ptr->race->ridx];
			dc = monster_x_char[m_ptr->race->ridx];

			/* Special handling of attrs and/or chars */
			if (da & 0x80) {
				/* Special attr/char codes */
				a = da;
				c = dc;
			} else if (OPT(purple_uniques) && 
					   rf_has(m_ptr->race->flags, RF_UNIQUE)) {
				/* Turn uniques purple if desired (violet, actually) */
				a = COLOUR_VIOLET;
				c = dc;
			} else if (rf_has(m_ptr->race->flags, RF_ATTR_MULTI) ||
					   rf_has(m_ptr->race->flags, RF_ATTR_FLICKER) ||
					   rf_has(m_ptr->race->flags, RF_ATTR_RAND)) {
				/* Multi-hued monster */
				a = m_ptr->attr ? m_ptr->attr : da;
				c = dc;
			} else if (!flags_test(m_ptr->race->flags, RF_SIZE,
								   RF_ATTR_CLEAR, RF_CHAR_CLEAR, FLAG_END)) {
				/* Normal monster (not "clear" in any way) */
				a = da;
				/* Desired attr & char. da is not used, should a be set to it?*/
				/*da = monster_x_attr[m_ptr->race->ridx];*/
				dc = monster_x_char[m_ptr->race->ridx];
				c = dc;
			} else if (a & 0x80) {
				/* Hack -- Bizarre grid under monster */
				a = da;
				c = dc;
			} else if (!rf_has(m_ptr->race->flags, RF_CHAR_CLEAR)) {
				/* Normal char, Clear attr, monster */
				c = dc;
			} else if (!rf_has(m_ptr->race->flags, RF_ATTR_CLEAR)) {
				/* Normal attr, Clear char, monster */
				a = da;
			}

			/* Store the drawing attr so we can use it elsewhere */
			m_ptr->attr = a;
		}
	} else if (g->is_player) {
		monster_race *r_ptr = &r_info[0];

		/* Get the "player" attr */
		a = monster_x_attr[r_ptr->ridx];
		if ((OPT(hp_changes_color)) && !(a & 0x80)) {
			switch(player->chp * 10 / player->mhp)
			{
			case 10:
			case  9: 
			{
				a = COLOUR_WHITE; 
				break;
			}
			case  8:
			case  7:
			{
				a = COLOUR_YELLOW;
				break;
			}
			case  6:
			case  5:
			{
				a = COLOUR_ORANGE;
				break;
			}
			case  4:
			case  3:
			{
				a = COLOUR_L_RED;
				break;
			}
			case  2:
			case  1:
			case  0:
			{
				a = COLOUR_RED;
				break;
			}
			default:
			{
				a = COLOUR_WHITE;
				break;
			}
			}
		}

		/* Get the "player" char */
		c = monster_x_char[r_ptr->ridx];
	} else if (g->trapborder && (g->f_idx) && !(g->first_kind)
			   && (use_graphics != GRAPHICS_NONE)) {
		/* No overlay is used, so we can use the trap border overlay */
		a = feat_x_attr[g->lighting][FEAT_DTRAP_WALL];
		c = feat_x_char[g->lighting][FEAT_DTRAP_WALL];
	}

	/* Result */
	(*ap) = a;
	(*cp) = c;
}


/**
 * Move the cursor to a given map location.
 */
static void move_cursor_relative_map(int y, int x)
{
	int ky, kx;

	term *old;

	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++) {
		term *t = angband_term[j];

		/* No window */
		if (!t) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_MAP))) continue;

		/* Location relative to panel */
		ky = y - t->offset_y;

		if (tile_height > 1)
			ky = tile_height * ky;

		/* Verify location */
		if ((ky < 0) || (ky >= t->hgt)) continue;

		/* Location relative to panel */
		kx = x - t->offset_x;

		if (tile_width > 1)
			kx = tile_width * kx;

		/* Verify location */
		if ((kx < 0) || (kx >= t->wid)) continue;

		/* Go there */
		old = Term;
		Term_activate(t);
		(void)Term_gotoxy(kx, ky);
		Term_activate(old);
	}
}


/**
 * Move the cursor to a given map location.
 *
 * The main screen will always be at least 24x80 in size.
 */
void move_cursor_relative(int y, int x)
{
	int ky, kx;
	int vy, vx;

	/* Move the cursor on map sub-windows */
	move_cursor_relative_map(y, x);

	/* Location relative to panel */
	ky = y - Term->offset_y;

	/* Verify location */
	if ((ky < 0) || (ky >= SCREEN_HGT)) return;

	/* Location relative to panel */
	kx = x - Term->offset_x;

	/* Verify location */
	if ((kx < 0) || (kx >= SCREEN_WID)) return;

	/* Location in window */
	vy = ky + ROW_MAP;

	/* Location in window */
	vx = kx + COL_MAP;

	if (tile_width > 1)
		vx += (tile_width - 1) * kx;

	if (tile_height > 1)
		vy += (tile_height - 1) * ky;

	/* Go there */
	(void)Term_gotoxy(vx, vy);
}


/**
 * Display an attr/char pair at the given map location
 *
 * Note the inline use of "panel_contains()" for efficiency.
 *
 * Note the use of "Term_queue_char()" for efficiency.
 */
static void print_rel_map(wchar_t c, byte a, int y, int x)
{
	int ky, kx;

	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++) {
		term *t = angband_term[j];

		/* No window */
		if (!t) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_MAP))) continue;

		/* Location relative to panel */
		ky = y - t->offset_y;

		if (tile_height > 1) {
			ky = tile_height * ky;
			if (ky + 1 >= t->hgt) continue;
		}

		/* Verify location */
		if ((ky < 0) || (ky >= t->hgt)) continue;

		/* Location relative to panel */
		kx = x - t->offset_x;

		if (tile_width > 1) {
			kx = tile_width * kx;
			if (kx + 1 >= t->wid) continue;
		}

		/* Verify location */
		if ((kx < 0) || (kx >= t->wid)) continue;

		/* Hack -- Queue it */
		Term_queue_char(t, kx, ky, a, c, 0, 0);

		if ((tile_width > 1) || (tile_height > 1))
			Term_big_queue_char(Term, kx, ky, a, c, 0, 0);
	}
}



/**
 * Display an attr/char pair at the given map location
 *
 * Note the inline use of "panel_contains()" for efficiency.
 *
 * Note the use of "Term_queue_char()" for efficiency.
 *
 * The main screen will always be at least 24x80 in size.
 */
void print_rel(wchar_t c, byte a, int y, int x)
{
	int ky, kx;
	int vy, vx;

	/* Print on map sub-windows */
	print_rel_map(c, a, y, x);

	/* Location relative to panel */
	ky = y - Term->offset_y;

	/* Verify location */
	if ((ky < 0) || (ky >= SCREEN_HGT)) return;

	/* Location relative to panel */
	kx = x - Term->offset_x;

	/* Verify location */
	if ((kx < 0) || (kx >= SCREEN_WID)) return;

	/* Get right position */
	vx = COL_MAP + (tile_width * kx);
	vy = ROW_MAP + (tile_height * ky);

	/* Hack -- Queue it */
	Term_queue_char(Term, vx, vy, a, c, 0, 0);

	if ((tile_width > 1) || (tile_height > 1))
		Term_big_queue_char(Term, vx, vy, a, c, 0, 0);
  
}


static void prt_map_aux(void)
{
	int a, ta;
	wchar_t c, tc;
	grid_data g;

	int y, x;
	int vy, vx;
	int ty, tx;

	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++) {
		term *t = angband_term[j];

		/* No window */
		if (!t) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_MAP))) continue;

		/* Assume screen */
		ty = t->offset_y + (t->hgt / tile_height);
		tx = t->offset_x + (t->wid / tile_width);

		/* Dump the map */
		for (y = t->offset_y, vy = 0; y < ty; vy++, y++) {
			if (vy + tile_height - 1 >= t->hgt) continue;
			for (x = t->offset_x, vx = 0; x < tx; vx++, x++) {
				/* Check bounds */
				if (!square_in_bounds(cave, y, x)) continue;
				if (vx + tile_width - 1 >= t->wid) continue;

				/* Determine what is there */
				map_info(y, x, &g);
				grid_data_as_text(&g, &a, &c, &ta, &tc);
				Term_queue_char(t, vx, vy, a, c, ta, tc);

				if ((tile_width > 1) || (tile_height > 1))
					Term_big_queue_char(t, vx, vy, 255, -1, 0, 0);
			}
		}
	}
}



/**
 * Redraw (on the screen) the current map panel
 *
 * Note the inline use of "light_spot()" for efficiency.
 *
 * The main screen will always be at least 24x80 in size.
 */
void prt_map(void)
{
	int a, ta;
	wchar_t c, tc;
	grid_data g;

	int y, x;
	int vy, vx;
	int ty, tx;

	/* Redraw map sub-windows */
	prt_map_aux();

	/* Assume screen */
	ty = Term->offset_y + SCREEN_HGT;
	tx = Term->offset_x + SCREEN_WID;

	/* Dump the map */
	for (y = Term->offset_y, vy = ROW_MAP; y < ty; vy+=tile_height, y++)
		for (x = Term->offset_x, vx = COL_MAP; x < tx; vx+=tile_width, x++) {
			/* Check bounds */
			if (!square_in_bounds(cave, y, x)) continue;

			/* Determine what is there */
			map_info(y, x, &g);
			grid_data_as_text(&g, &a, &c, &ta, &tc);

			/* Hack -- Queue it */
			Term_queue_char(Term, vx, vy, a, c, ta, tc);

			if ((tile_width > 1) || (tile_height > 1))
				Term_big_queue_char(Term, vx, vy, a, c, COLOUR_WHITE, L' ');
		}
}

/**
 * Display a "small-scale" map of the dungeon in the active Term.
 *
 * Note that this function must "disable" the special lighting effects so
 * that the "priority" function will work.
 *
 * Note the use of a specialized "priority" function to allow this function
 * to work with any graphic attr/char mappings, and the attempts to optimize
 * this function where possible.
 *
 * If "cy" and "cx" are not NULL, then returns the screen location at which
 * the player was displayed, so the cursor can be moved to that location,
 * and restricts the horizontal map size to SCREEN_WID.  Otherwise, nothing
 * is returned (obviously), and no restrictions are enforced.
 */
void display_map(int *cy, int *cx)
{
	int py = player->py;
	int px = player->px;

	int map_hgt, map_wid;
	int row, col;

	int x, y;
	grid_data g;

	int a, ta;
	wchar_t c, tc;

	byte tp;

	monster_race *r_ptr = &r_info[0];

	/* Priority array */
	byte **mp = mem_zalloc(cave->height * sizeof(byte*));
	for (y = 0; y < cave->height; y++)
		mp[y] = mem_zalloc(cave->width * sizeof(byte));

	/* Desired map height */
	map_hgt = Term->hgt - 2;
	map_wid = Term->wid - 2;

	/* Prevent accidents */
	if (map_hgt > cave->height) map_hgt = cave->height;
	if (map_wid > cave->width) map_wid = cave->width;

	/* Prevent accidents */
	if ((map_wid < 1) || (map_hgt < 1)) {
		for (y = 0; y < cave->height; y++)
			mem_free(mp[y]);
		mem_free(mp);
		return;
	}

	/* Nothing here */
	a = COLOUR_WHITE;
    c = L' ';
	ta = COLOUR_WHITE;
	tc = L' ';

	/* Draw a box around the edge of the term */
	window_make(0, 0, map_wid + 1, map_hgt + 1);

	/* Analyze the actual map */
	for (y = 0; y < cave->height; y++)
		for (x = 0; x < cave->width; x++) {
			row = (y * map_hgt / cave->height);
			col = (x * map_wid / cave->width);

			if (tile_width > 1)
				col = col - (col % tile_width);
			if (tile_height > 1)
				row = row - (row % tile_height);

			/* Get the attr/char at that map location */
			map_info(y, x, &g);
			grid_data_as_text(&g, &a, &c, &ta, &tc);

			/* Get the priority of that attr/char */
			tp = f_info[g.f_idx].priority;

			/* Stuff on top of terrain gets higher priority */
			if ((a != ta) || (c != tc)) tp = 20;

			/* Save "best" */
			if (mp[row][col] < tp) {
				/* Hack - make every grid on the map lit */
				g.lighting = LIGHTING_LIT;
				grid_data_as_text(&g, &a, &c, &ta, &tc);

				Term_queue_char(Term, col + 1, row + 1, a, c, ta, tc);

				if ((tile_width > 1) || (tile_height > 1))
					Term_big_queue_char(Term, col + 1, row + 1, 255, -1, 0, 0);

				/* Save priority */
				mp[row][col] = tp;
			}
		}

	/*** Display the player ***/

	/* Player location */
	row = (py * map_hgt / cave->height);
	col = (px * map_wid / cave->width);

	if (tile_width > 1)
		col = col - (col % tile_width);
	if (tile_height > 1)
		row = row - (row % tile_height);

	/* Get the "player" tile */
	ta = monster_x_attr[r_ptr->ridx];
	tc = monster_x_char[r_ptr->ridx];

	/* Draw the player */
	Term_putch(col + 1, row + 1, ta, tc);

	if ((tile_width > 1) || (tile_height > 1))
		Term_big_putch(col + 1, row + 1, ta, tc);
  
	/* Return player location */
	if (cy != NULL) (*cy) = row + 1;
	if (cx != NULL) (*cx) = col + 1;

	for (y = 0; y < cave->height; y++)
		mem_free(mp[y]);
	mem_free(mp);
}


/*
 * Display a "small-scale" map of the dungeon.
 *
 * Note that the "player" is always displayed on the map.
 */
void do_cmd_view_map(void)
{
	int cy, cx;
	byte w, h;
	const char *prompt = "Hit any key to continue";
	if (Term->view_map_hook) {
		(*(Term->view_map_hook))(Term);
		return;
	}
	/* Save screen */
	screen_save();

	/* Note */
	prt("Please wait...", 0, 0);

	/* Flush */
	Term_fresh();

	/* Clear the screen */
	Term_clear();

	/* store the tile multipliers */
	w = tile_width;
	h = tile_height;
	tile_width = 1;
	tile_height = 1;

	/* Display the map */
	display_map(&cy, &cx);

	/* Show the prompt */
	put_str(prompt, Term->hgt - 1, Term->wid / 2 - strlen(prompt) / 2);

	/* Highlight the player */
	Term_gotoxy(cx, cy);

	/* Get any key */
	(void)anykey();

	/* Restore the tile multipliers */
	tile_width = w;
	tile_height = h;

	/* Load screen */
	screen_load();
}



#ifndef _SWAY_LAYOUTS_H
#define _SWAY_LAYOUTS_H

#include <stdbool.h>
#include <wlr/util/box.h>
#include "list.h"

#define SWAY_LAYOUT_RATIO_DEFAULT 0.55
#define SWAY_LAYOUT_RATIO_MIN 0.1
#define SWAY_LAYOUT_RATIO_MAX 0.9

struct sway_workspace;

struct sway_layout {
	const char *name;
	void (*compute)(int n, const struct wlr_box *area, int gap, double ratio,
		struct wlr_box *out);
};

extern const struct sway_layout sway_layout_none;
extern const struct sway_layout sway_layout_master_stack;
extern const struct sway_layout sway_layout_grid;

const struct sway_layout *sway_layout_find(const char *name);

const struct sway_layout *sway_layout_next(const struct sway_layout *current);

const struct sway_layout *sway_layout_effective(const struct sway_workspace *ws);

double sway_layout_ratio(const struct sway_workspace *ws);

bool sway_layout_apply(list_t *children, struct wlr_box *parent);

#endif

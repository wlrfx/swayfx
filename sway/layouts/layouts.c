#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "sway/config.h"
#include "sway/layouts.h"
#include "sway/tree/container.h"
#include "sway/tree/workspace.h"

#define MIN_SANE_SIZE 10

const struct sway_layout sway_layout_none = {
	.name = "none",
};

static const struct sway_layout *layouts[] = {
	&sway_layout_none,
	&sway_layout_master_stack,
	&sway_layout_grid,
};

#define LAYOUT_COUNT (sizeof(layouts) / sizeof(layouts[0]))

const struct sway_layout *sway_layout_find(const char *name) {
	for (size_t i = 0; i < LAYOUT_COUNT; ++i) {
		if (strcasecmp(layouts[i]->name, name) == 0) {
			return layouts[i];
		}
	}
	return NULL;
}

const struct sway_layout *sway_layout_next(const struct sway_layout *current) {
	size_t index = 0;
	for (size_t i = 0; i < LAYOUT_COUNT; ++i) {
		if (layouts[i] == current) {
			index = i;
		}
	}
	return layouts[(index + 1) % LAYOUT_COUNT];
}

const struct sway_layout *sway_layout_effective(const struct sway_workspace *ws) {
	if (ws && ws->layout_algo) {
		return ws->layout_algo;
	}
	return config->layout_algo;
}

double sway_layout_ratio(const struct sway_workspace *ws) {
	double ratio = ws && ws->layout_ratio > 0 ?
		ws->layout_ratio : config->layout_ratio;
	if (!(ratio >= SWAY_LAYOUT_RATIO_MIN && ratio <= SWAY_LAYOUT_RATIO_MAX)) {
		ratio = SWAY_LAYOUT_RATIO_DEFAULT;
	}
	return ratio;
}

bool sway_layout_apply(list_t *children, struct wlr_box *parent) {
	if (!children->length) {
		return false;
	}

	struct sway_container *first = children->items[0];
	if (first->pending.parent != NULL) {
		return false;
	}

	struct sway_workspace *ws = first->pending.workspace;
	const struct sway_layout *layout = sway_layout_effective(ws);
	if (!layout || !layout->compute) {
		return false;
	}

	struct wlr_box *rects = calloc(children->length, sizeof(*rects));
	if (!rects) {
		return false;
	}

	layout->compute(children->length, parent, ws ? ws->gaps_inner : 0,
		sway_layout_ratio(ws), rects);

	for (int i = 0; i < children->length; ++i) {
		struct sway_container *child = children->items[i];
		child->child_total_width = parent->width;
		child->child_total_height = parent->height;
		child->pending.x = rects[i].x;
		child->pending.y = rects[i].y;
		child->pending.width = rects[i].width;
		child->pending.height = rects[i].height;
		if (child->pending.width < MIN_SANE_SIZE ||
				child->pending.height < MIN_SANE_SIZE) {
			child->pending.width = 0;
			child->pending.height = 0;
		}
	}

	free(rects);
	return true;
}

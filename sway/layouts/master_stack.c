#include "sway/layouts.h"

static void compute(int n, const struct wlr_box *area, int gap, double ratio,
		struct wlr_box *out) {
	if (n == 1) {
		out[0] = *area;
		return;
	}

	int usable_w = area->width - gap;
	if (usable_w < 0) {
		usable_w = 0;
	}
	int master_w = (int)(usable_w * ratio);
	int right = area->x + area->width;
	int bottom = area->y + area->height;
	int stack_x = area->x + master_w + gap;
	if (stack_x > right) {
		stack_x = right;
	}
	int stack_w = right - stack_x;

	int stack_n = n - 1;
	int stack_h = (area->height - gap * (stack_n - 1)) / stack_n;
	if (stack_h < 0) {
		stack_h = 0;
	}

	out[0] = (struct wlr_box){area->x, area->y, master_w, area->height};
	for (int i = 1; i < n; ++i) {
		int y = area->y + (i - 1) * (stack_h + gap);
		if (y > bottom) {
			y = bottom;
		}
		int h = i == n - 1 ? bottom - y : stack_h;
		if (y + h > bottom) {
			h = bottom - y;
		}
		out[i] = (struct wlr_box){stack_x, y, stack_w, h};
	}
}

const struct sway_layout sway_layout_master_stack = {
	.name = "master_stack",
	.compute = compute,
};

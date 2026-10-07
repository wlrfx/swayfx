#include "sway/layouts.h"

static void compute(int n, const struct wlr_box *area, int gap, double ratio,
		struct wlr_box *out) {
	(void)ratio;

	int cols = 1;
	while (cols * cols < n) {
		cols++;
	}
	int rows = (n + cols - 1) / cols;
	int right = area->x + area->width;
	int bottom = area->y + area->height;

	int row_h = (area->height - gap * (rows - 1)) / rows;
	if (row_h < 0) {
		row_h = 0;
	}

	for (int i = 0; i < n; ++i) {
		int row = i / cols;
		int col = i % cols;
		int in_row = row == rows - 1 ? n - cols * (rows - 1) : cols;

		int cell_w = (area->width - gap * (in_row - 1)) / in_row;
		if (cell_w < 0) {
			cell_w = 0;
		}

		int x = area->x + col * (cell_w + gap);
		if (x > right) {
			x = right;
		}
		int w = col == in_row - 1 ? right - x : cell_w;
		if (x + w > right) {
			w = right - x;
		}

		int y = area->y + row * (row_h + gap);
		if (y > bottom) {
			y = bottom;
		}
		int h = row == rows - 1 ? bottom - y : row_h;
		if (y + h > bottom) {
			h = bottom - y;
		}

		out[i] = (struct wlr_box){x, y, w, h};
	}
}

const struct sway_layout sway_layout_grid = {
	.name = "grid",
	.compute = compute,
};

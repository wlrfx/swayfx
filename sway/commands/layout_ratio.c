#include <math.h>
#include <stdlib.h>
#include "sway/commands.h"
#include "sway/config.h"
#include "sway/layouts.h"
#include "sway/tree/arrange.h"
#include "sway/tree/workspace.h"

struct cmd_results *cmd_layout_ratio(int argc, char **argv) {
	struct cmd_results *error = checkarg(argc, "layout_ratio", EXPECTED_EQUAL_TO, 1);
	if (error) {
		return error;
	}

	char *end;
	double value = strtod(argv[0], &end);
	if (end == argv[0] || *end != '\0' || !isfinite(value)) {
		return cmd_results_new(CMD_INVALID,
			"Expected a number, got '%s'", argv[0]);
	}

	struct sway_workspace *ws = config->handler_context.workspace;

	double ratio = value;
	if (argv[0][0] == '+' || argv[0][0] == '-') {
		ratio = sway_layout_ratio(ws) + value;
		if (ratio < SWAY_LAYOUT_RATIO_MIN) {
			ratio = SWAY_LAYOUT_RATIO_MIN;
		}
		if (ratio > SWAY_LAYOUT_RATIO_MAX) {
			ratio = SWAY_LAYOUT_RATIO_MAX;
		}
	} else if (ratio < SWAY_LAYOUT_RATIO_MIN || ratio > SWAY_LAYOUT_RATIO_MAX) {
		return cmd_results_new(CMD_INVALID,
			"layout_ratio must be between %.1f and %.1f",
			SWAY_LAYOUT_RATIO_MIN, SWAY_LAYOUT_RATIO_MAX);
	}

	if (ws) {
		ws->layout_ratio = ratio;
	} else {
		config->layout_ratio = ratio;
	}

	arrange_root();
	return cmd_results_new(CMD_SUCCESS, NULL);
}

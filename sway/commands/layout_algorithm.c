#include <string.h>
#include <strings.h>
#include "sway/commands.h"
#include "sway/config.h"
#include "sway/layouts.h"
#include "sway/tree/arrange.h"
#include "sway/tree/workspace.h"

struct cmd_results *cmd_layout_algorithm(int argc, char **argv) {
	struct cmd_results *error =
		checkarg(argc, "layout_algorithm", EXPECTED_EQUAL_TO, 1);
	if (error) {
		return error;
	}

	struct sway_workspace *ws = config->handler_context.workspace;

	const struct sway_layout *layout;
	if (strcasecmp(argv[0], "next") == 0) {
		layout = sway_layout_next(sway_layout_effective(ws));
	} else {
		layout = sway_layout_find(argv[0]);
	}
	if (!layout) {
		return cmd_results_new(CMD_INVALID,
			"Unknown layout algorithm '%s'", argv[0]);
	}

	if (ws) {
		ws->layout_algo = layout;
	} else {
		config->layout_algo = layout;
	}

	arrange_root();
	return cmd_results_new(CMD_SUCCESS, NULL);
}

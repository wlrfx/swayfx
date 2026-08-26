#include <strings.h>
#include "sway/commands.h"
#include "sway/config.h"

struct cmd_results *cmd_workspace_effect(int argc, char **argv) {
	struct cmd_results *error = NULL;
	if ((error = checkarg(argc, "workspace_effect", EXPECTED_EQUAL_TO, 1))) {
		return error;
	}
	if (strcasecmp(argv[0], "fade") == 0) {
		config->workspace_effect = WORKSPACE_EFFECT_FADE;
	} else if (strcasecmp(argv[0], "slide") == 0) {
		config->workspace_effect = WORKSPACE_EFFECT_SLIDE;
	} else if (strcasecmp(argv[0], "none") == 0) {
		config->workspace_effect = WORKSPACE_EFFECT_NONE;
	} else {
		return cmd_results_new(CMD_INVALID,
				"Expected 'workspace_effect <fade|slide|none>'");
	}
	return cmd_results_new(CMD_SUCCESS, NULL);
}

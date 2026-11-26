#include <strings.h>
#include "sway/animation_manager.h"
#include "sway/commands.h"

static const char *easing_names[] = {
	[EASING_LINEAR] = "linear",
	[EASING_QUAD] = "quad",
	[EASING_CUBIC] = "cubic",
	[EASING_BACK] = "back",
};

struct cmd_results *cmd_animation_easing(int argc, char **argv) {
	struct cmd_results *error = NULL;
	if ((error = checkarg(argc, "animation_easing", EXPECTED_EQUAL_TO, 1))) {
		return error;
	}

	for (size_t i = 0; i < sizeof(easing_names) / sizeof(easing_names[0]); i++) {
		if (strcasecmp(argv[0], easing_names[i]) == 0) {
			config->animation_easing = i;
			return cmd_results_new(CMD_SUCCESS, NULL);
		}
	}

	return cmd_results_new(CMD_INVALID,
		"Expected animation_easing to be one of: linear, quad, cubic, back");
}
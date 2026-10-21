#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "sway/commands.h"
#include "sway/config.h"
#include "sway/tree/arrange.h"
#include "sway/tree/container.h"

static struct cmd_results *label_cmd_enable(struct sway_container *container,
		int argc, char **argv) {
	if (strcmp(argv[0], "enable") == 0) {
		container->label_enabled = true;
	} else if (strcmp(argv[0], "disable") == 0) {
		container->label_enabled = false;
	} else if (strcmp(argv[0], "toggle") == 0) {
		container->label_enabled = !container->label_enabled;
	} else {
		return cmd_results_new(CMD_INVALID,
				"Expected 'label enable|disable|toggle'");
	}

	// Autohide arms on a focus-GAIN edge (was_focused: false -> true) —
	// container_update() already tracks was_focused continuously,
	// regardless of label_enabled, so enabling the label on a window
	// that's already focused sees no such edge and would otherwise never
	// start the countdown until an actual focus loss/regain cycle. Arm it
	// here explicitly for that case (a no-op via its own guards if the
	// container isn't focused, autohide is off, or label ended up
	// disabled).
	if (container->label_enabled) {
		container_label_rearm_autohide(container);
	}
	return NULL;
}

static struct cmd_results *label_cmd_position(struct sway_container *container,
		int argc, char **argv) {
	if (argc < 3) {
		return cmd_results_new(CMD_INVALID,
				"Expected 'label position <top|bottom> <left|center|right>'");
	}
	if (strcmp(argv[1], "top") == 0) {
		container->label_edge = LABEL_EDGE_TOP;
	} else if (strcmp(argv[1], "bottom") == 0) {
		container->label_edge = LABEL_EDGE_BOTTOM;
	} else {
		return cmd_results_new(CMD_INVALID,
				"Expected 'label position <top|bottom> <left|center|right>'");
	}
	if (strcmp(argv[2], "left") == 0) {
		container->label_align = LABEL_ALIGN_LEFT;
	} else if (strcmp(argv[2], "center") == 0) {
		container->label_align = LABEL_ALIGN_CENTER;
	} else if (strcmp(argv[2], "right") == 0) {
		container->label_align = LABEL_ALIGN_RIGHT;
	} else {
		return cmd_results_new(CMD_INVALID,
				"Expected 'label position <top|bottom> <left|center|right>'");
	}
	return NULL;
}

static bool parse_label_max_width(const char *arg, int *px, float *percent,
		bool *is_percent) {
	char *end;
	float value = strtof(arg, &end);
	// strtof() happily parses "inf"/"nan" (case-insensitively, with an
	// optional sign) and neither compares less than 0, so `value < 0`
	// alone lets them through; casting a non-finite float to int below is
	// undefined behavior.
	if (end == arg || !isfinite(value) || value < 0) {
		return false;
	}
	if (*end == '%' && *(end + 1) == '\0') {
		*is_percent = true;
		*percent = value;
		return true;
	}
	if (*end == '\0') {
		*is_percent = false;
		*px = (int)value;
		return true;
	}
	return false;
}

static struct cmd_results *label_cmd_max_width(struct sway_container *container,
		int argc, char **argv) {
	if (argc < 2) {
		return cmd_results_new(CMD_INVALID,
				"Expected 'label max_width <px>|<percent>%%'");
	}
	int px = 0;
	float percent = 0;
	bool is_percent = false;
	if (!parse_label_max_width(argv[1], &px, &percent, &is_percent)) {
		return cmd_results_new(CMD_INVALID,
				"Invalid max_width value '%s' (expected e.g. '200' or '50%%')",
				argv[1]);
	}
	container->label_max_width_is_percent = is_percent;
	if (is_percent) {
		container->label_max_width_percent = percent;
	} else {
		container->label_max_width = px;
	}
	return NULL;
}

static struct cmd_results *label_cmd_corner_radius(struct sway_container *container,
		int argc, char **argv) {
	if (argc < 2) {
		return cmd_results_new(CMD_INVALID,
				"Expected 'label corner_radius <px>|match'");
	}
	if (strcmp(argv[1], "match") == 0) {
		container->label_corner_radius_match_window = true;
		return NULL;
	}
	int value;
	// Reuse the window corner_radius command's own parser/range (0-99)
	// rather than duplicating it — a future change to the accepted range
	// would otherwise only apply to one of the two commands.
	if (!cmd_corner_radius_parse_value(argv[1], &value)) {
		return cmd_results_new(CMD_INVALID,
				"Invalid corner_radius value '%s' (expected 0-99 or 'match')",
				argv[1]);
	}
	container->label_corner_radius_match_window = false;
	container->label_corner_radius = value;
	return NULL;
}

static struct cmd_results *label_cmd_autohide(struct sway_container *container,
		int argc, char **argv) {
	if (argc < 2) {
		return cmd_results_new(CMD_INVALID,
				"Expected 'label autohide off|<ms>'");
	}
	if (strcmp(argv[1], "off") == 0) {
		container->label_autohide_ms = 0;
	} else {
		char *end;
		int value = strtol(argv[1], &end, 10);
		if (*end != '\0' || value < 0) {
			return cmd_results_new(CMD_INVALID,
					"Invalid autohide value '%s' (expected 'off' or a positive number of ms)",
					argv[1]);
		}
		container->label_autohide_ms = value;
	}

	// Re-sync any in-flight timer/fade to the new setting immediately —
	// otherwise a countdown already running against the OLD value keeps
	// firing (or not firing) on its own schedule regardless of what was
	// just configured. Cancel + restore first, then re-arm fresh only if
	// the new setting and current focus state actually call for it.
	container_label_restore_visibility(container);
	container_label_rearm_autohide(container);
	return NULL;
}

static struct cmd_results *label_cmd_avoid_cursor(struct sway_container *container,
		int argc, char **argv) {
	if (argc < 2) {
		return cmd_results_new(CMD_INVALID,
				"Expected 'label avoid_cursor on|off'");
	}
	if (strcmp(argv[1], "on") == 0) {
		container_set_label_avoid_cursor(container, true);
	} else if (strcmp(argv[1], "off") == 0) {
		container_set_label_avoid_cursor(container, false);
	} else {
		return cmd_results_new(CMD_INVALID,
				"Expected 'label avoid_cursor on|off'");
	}
	return NULL;
}

struct cmd_results *cmd_label(int argc, char **argv) {
	struct cmd_results *error = NULL;
	if ((error = checkarg(argc, "label", EXPECTED_AT_LEAST, 1))) {
		return error;
	}

	struct sway_container *container = config->handler_context.container;
	if (!container || !container->view) {
		return cmd_results_new(CMD_INVALID, "Only views can have a label");
	}

	if (strcmp(argv[0], "enable") == 0 || strcmp(argv[0], "disable") == 0
			|| strcmp(argv[0], "toggle") == 0) {
		error = label_cmd_enable(container, argc, argv);
	} else if (strcmp(argv[0], "position") == 0) {
		error = label_cmd_position(container, argc, argv);
	} else if (strcmp(argv[0], "max_width") == 0) {
		error = label_cmd_max_width(container, argc, argv);
	} else if (strcmp(argv[0], "corner_radius") == 0) {
		error = label_cmd_corner_radius(container, argc, argv);
	} else if (strcmp(argv[0], "autohide") == 0) {
		error = label_cmd_autohide(container, argc, argv);
	} else if (strcmp(argv[0], "avoid_cursor") == 0) {
		error = label_cmd_avoid_cursor(container, argc, argv);
	} else {
		return cmd_results_new(CMD_INVALID,
				"Expected 'label enable|disable|toggle|position|max_width|"
				"corner_radius|autohide|avoid_cursor ...'");
	}
	if (error) {
		return error;
	}

	// Toggling the label changes whether this container's B_NORMAL top
	// edge reserves titlebar height (see container_set_geometry_from_content()
	// and view_autoconfigure()) — for a floating window, resize the frame
	// to match rather than silently changing the content area, same as
	// cmd_border does for its own decoration changes.
	if (container_is_floating(container)) {
		container_set_geometry_from_content(container);
	}

	arrange_container(container);

	return cmd_results_new(CMD_SUCCESS, NULL);
}

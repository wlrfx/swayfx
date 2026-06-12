#ifndef _SWAY_ANIMATION_MANAGER_H
#define _SWAY_ANIMATION_MANAGER_H

#include <stdbool.h>
#include <wayland-util.h>

struct sway_container;
struct sway_server;

// TODO: make animation just a pointer to progress, make multiplier and callback private
struct animation {
	struct wl_list link;
	float progress;
	void *data;
	float multiplier;
	bool initialized;
	void (*update)(void *);
	void (*complete)(void *);
};

void animation_manager_init(struct sway_server *server);

struct animation init_animation(void *data);

void refresh_animation_manager_timing();

void add_animation(struct animation *animation, void (*update_callback)(void *),
	void (*complete_callback)(void *));

void finish_animation(struct animation *animation);

void start_animations();

float get_animated_value(float from, float to, const struct animation *animation);

#endif


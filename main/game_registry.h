#pragma once
#include <stddef.h>
#include <stdbool.h>
#include "demo.h"
/* Append games here through game_registry.c. Callbacks follow demo.h locking
 * and stop-before-destroy contract. Games consume badge_theme.h under the LVGL
 * lock and apply palette revisions during UI refresh. No UI or navigation edits are required. */
typedef struct {
    const char *id;
    const char *description;
    const char *category;
    /* Optional nested-page back; true consumes it, false exits to the library.
     * Called without LVGL lock from the lifecycle task. */
    bool (*back)(void);
    demo_entry_t lifecycle;
} badge_game_t;
extern const badge_game_t BADGE_GAMES_REGISTRY[];
extern const size_t BADGE_GAME_COUNT;

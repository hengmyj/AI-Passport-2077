#pragma once
#include "radio_player.h"
// Immutable curated stations stay in flash; only local results occupy RAM.
extern const RadioStation RADIO_PRESETS[];
extern const std::size_t RADIO_PRESET_COUNT;

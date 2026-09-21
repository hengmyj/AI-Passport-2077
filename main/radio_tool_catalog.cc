#include "radio_tool_catalog.h"
#include "radio_presets.h"
extern "C" unsigned radio_tool_count(void){return RADIO_PRESET_COUNT;}
extern "C" const char *radio_tool_name(unsigned id){return id<RADIO_PRESET_COUNT?RADIO_PRESETS[id].name:"";}

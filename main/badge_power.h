#pragma once
#include <stdbool.h>
#include "esp_err.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Display transitions belong ONLY to the shell. A joined, exclusive audio
 * worker owns enter/tick/leave and its separate performance leases. */
esp_err_t badge_power_init(void);
void badge_power_set_timeout(unsigned choice);
void badge_power_display_tick(bool keep_awake,bool keep_cpu);
void badge_power_enter(void);
/* Releases ownership even on error; shell retries failed suspend safely. */
esp_err_t badge_power_leave(void);
void badge_power_activity(void);
void badge_power_audio_activity(void);
void badge_power_tick(bool busy,bool audio_busy);
/* Mini apps: retain playback performance, sleep after 15 silent seconds;
 * does not mark the conversation busy or keep the display lit. */
void badge_power_audio_tick(bool audio_busy);
bool badge_power_screen_off(void);
typedef struct {bool screen_off,configured,display_performance,audio_performance,
    audio_owner,voice_listening,audio_sleeping;} badge_power_status_t;
/* Read-only diagnostic snapshot; does not wake the display. */
badge_power_status_t badge_power_status(void);
/* Called only by the audio/power owner. Keep RX active during screen sleep. */
void badge_power_set_local_listening(bool enabled);
#ifdef __cplusplus
}
#endif

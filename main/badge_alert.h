#pragma once
#include <stdbool.h>
/* Navigation task only. Start after stopping all app audio producers. */
bool badge_alert_start(const char *text);
bool badge_alert_dismiss(void);

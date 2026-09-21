#pragma once
#include "esp_http_client.h"
/* Worker-owned clients only. Applies redirects for the streaming open API. */
bool radio_http_open(esp_http_client_handle_t client,unsigned redirects);

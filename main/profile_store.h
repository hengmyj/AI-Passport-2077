#pragma once
#include "profile_format.h"
#include "esp_err.h"
esp_err_t profile_store_init(void);
unsigned profile_store_revision(void);
const uint8_t *profile_store_card(void);
void profile_store_displayed(unsigned revision);
const char *profile_store_json(size_t *size);
esp_err_t profile_store_read(profile_asset_t asset,size_t offset,void *data,size_t size);
esp_err_t profile_store_begin(const char *json,size_t size);
esp_err_t profile_store_append(size_t offset,const void *data,size_t size);
esp_err_t profile_store_commit(void);
esp_err_t profile_store_clear(void);
void profile_store_abort(void);

unsigned profile_store_version(void);
const uint8_t *profile_store_asset(profile_asset_t asset);
esp_err_t profile_store_begin_v2(const char *json,size_t size,bool qr);

/* Serialize these calls with the shared protocol lock. IDs are zero-based. */
unsigned profile_store_count(void);
unsigned profile_store_badge(void);
bool profile_store_ready(void);
bool profile_store_refresh_pending(void);
esp_err_t profile_store_select(unsigned id);
const uint8_t *profile_store_card_for(unsigned id);
const char *profile_store_json_for(unsigned id,size_t *size);
unsigned profile_store_version_for(unsigned id);
const uint8_t *profile_store_asset_for(unsigned id,profile_asset_t asset);
esp_err_t profile_store_read_for(unsigned id,profile_asset_t asset,size_t offset,void *data,size_t size);
esp_err_t profile_store_begin_for(unsigned id,const char *json,size_t size,bool v2,bool qr);
esp_err_t profile_store_clear_for(unsigned id);

unsigned profile_store_revision_for(unsigned id);

esp_err_t profile_store_begin_format(unsigned id,const char *json,size_t size,unsigned format,bool qr);
/* Atomic bank clone: source assets remain valid until the commit marker. */
esp_err_t profile_store_patch(unsigned id,const char *json,size_t size,void (*patch)(size_t,uint8_t *,size_t,void *),void *context);

#define main profile_store_tests_main
#include "test_profile_store.c"
#undef main
#include "profile_edit.h"
#include "lvgl.h"
bool profile_protocol_lock(void){return true;}
void profile_protocol_unlock(void){}
int main(void){
    profile_store_tests_main();
    profile_store_displayed(profile_store_revision());
    const char *json="{\"name\":\"FOLO MAKER\",\"department\":\"设计部\",\"title\":\"工程师\",\"alias\":\"ART\",\"theme\":{\"background\":\"#08090b\",\"text\":\"#e8e6df\",\"muted\":\"#a69c9f\"}}";
    assert(profile_store_begin_format(0,json,strlen(json),4,true)==ESP_OK);upload_bytes(0x77,PROFILE_V4_BYTES);assert(profile_store_commit()==ESP_OK);
    profile_store_displayed(profile_store_revision());unsigned rev=profile_store_revision_for(0);
    assert(profile_edit_locked(0,rev+1,2,"产品设计师")==ESP_ERR_INVALID_STATE);
    assert(profile_edit_locked(0,rev,2,"产品设计师")==ESP_OK);
    const uint8_t *pixels=profile_store_card_for(0);unsigned changed=0;
    for(size_t i=0;i<PROFILE_V4_BYTES;i++){size_t y=i/2/84;if(y>=90&&y<122){if(pixels[i]!=0x77)changed++;}else assert(pixels[i]==0x77);}
    assert(changed>5000);cJSON *p=profile_edit_get(0);assert(p&&!strcmp(cJSON_GetObjectItemCaseSensitive(p,"title")->valuestring,"产品设计师"));cJSON_Delete(p);
    profile_store_displayed(profile_store_revision());rev=profile_store_revision_for(0);
    assert(profile_edit_locked(0,rev,0,"\xf4\x8f\xbf\xbf")==ESP_ERR_INVALID_ARG);assert(profile_store_revision_for(0)==rev);
    assert(profile_edit_locked(0,rev,0,"测试姓名")==ESP_OK);profile_store_displayed(profile_store_revision());
    rev=profile_store_revision_for(0);assert(profile_edit_locked(0,rev,1,"")==ESP_OK);profile_store_displayed(profile_store_revision());
    rev=profile_store_revision_for(0);fail_header=true;assert(profile_edit_locked(0,rev,2,"失败测试")!=ESP_OK);fail_header=false;
    assert(profile_store_init()==ESP_OK);p=profile_edit_get(0);assert(!strcmp(cJSON_GetObjectItemCaseSensitive(p,"title")->valuestring,"产品设计师"));cJSON_Delete(p);
    assert(profile_store_asset_for(0,PROFILE_ASSET_QR)[0]==0x77&&profile_store_asset_for(0,PROFILE_ASSET_AVATAR)[0]==0x77);
    puts("Profile edit PASS: real Chinese glyphs, stale revision, unsupported characters, persistent JSON+pixels, asset preservation and rollback");return 0;
}

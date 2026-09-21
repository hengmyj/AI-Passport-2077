#include "profile_edit.h"
#include "profile_protocol.h"
#include "profile_store.h"
#include "profile_text.h"
#include "lvgl.h"
#include <stdlib.h>
#include <string.h>
LV_FONT_DECLARE(font_xiaozhi_14);
static const char *fields[]={"name","department","title"};
static bool glyph(uint32_t cp,profile_glyph_t *out){
    const lv_font_fmt_txt_dsc_t *d=font_xiaozhi_14.dsc;const lv_font_fmt_txt_cmap_t *m=&d->cmaps[0];
    if(d->bpp!=1||d->bitmap_format!=0||cp<m->range_start||cp-m->range_start>=m->range_length)return false;
    unsigned lo=0,hi=m->list_length;uint32_t value=cp-m->range_start;
    while(lo<hi){unsigned mid=lo+(hi-lo)/2;if(m->unicode_list[mid]<value)lo=mid+1;else hi=mid;}
    if(lo>=m->list_length||m->unicode_list[lo]!=value)return false;
    const lv_font_fmt_txt_glyph_dsc_t *g=&d->glyph_dsc[m->glyph_id_start+lo];
    *out=(profile_glyph_t){d->glyph_bitmap+g->bitmap_index,g->box_w,g->box_h,(g->adv_w+8)/16,g->ofs_x,g->ofs_y};return true;
}
static uint16_t color(cJSON *p,const char *key,uint32_t fallback){
    cJSON *v=cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(p,"theme"),key);
    if(cJSON_IsString(v)&&strlen(v->valuestring)==7&&v->valuestring[0]=='#')fallback=(uint32_t)strtoul(v->valuestring+1,NULL,16);
    return (uint16_t)(((fallback>>8)&0xf800)|((fallback>>5)&0x7e0)|((fallback>>3)&31));
}
cJSON *profile_edit_get(unsigned id){
    if(!profile_protocol_lock())return NULL;
    if(id==5)id=profile_store_badge();
    size_t n=0;const char *raw=profile_store_json_for(id,&n);cJSON *p=cJSON_ParseWithLength(raw,n),*out=cJSON_CreateObject();
    if(out){cJSON_AddNumberToObject(out,"number",id+1);cJSON_AddNumberToObject(out,"revision",profile_store_revision_for(id));
        cJSON_AddBoolToObject(out,"configured",id<profile_store_count()&&profile_store_card_for(id)!=NULL);
        cJSON_AddBoolToObject(out,"editable",profile_store_version_for(id)==4);
        for(unsigned i=0;i<3;i++){cJSON *v=cJSON_GetObjectItemCaseSensitive(p,fields[i]);cJSON_AddStringToObject(out,fields[i],cJSON_IsString(v)?v->valuestring:"");}
    }cJSON_Delete(p);profile_protocol_unlock();return out;
}
esp_err_t profile_edit_locked(unsigned id,unsigned revision,unsigned field,const char *text){
    if(id>=profile_store_count()||field>2||!profile_store_card_for(id)||profile_store_version_for(id)!=4||!profile_store_ready()||profile_store_revision_for(id)!=revision)return ESP_ERR_INVALID_STATE;
    size_t n;const char *raw=profile_store_json_for(id,&n);cJSON *p=cJSON_ParseWithLength(raw,n);if(!p)return ESP_ERR_NO_MEM;
    profile_text_patch_t *patch=malloc(sizeof(*patch));esp_err_t e=ESP_ERR_INVALID_ARG;
    if(!patch){cJSON_Delete(p);return ESP_ERR_NO_MEM;}
    if(profile_text_prepare(patch,field,text,color(p,"background",0x08090b),color(p,field==2?"muted":"text",field==2?0xa69c9f:0xe8e6df),glyph)){
        cJSON *old=cJSON_GetObjectItemCaseSensitive(p,fields[field]);
        if(cJSON_IsString(old)&&!strcmp(old->valuestring,text))e=ESP_OK;
        else if(cJSON_SetValuestring(old,text)){
            char *json=cJSON_PrintUnformatted(p);if(json){e=profile_store_patch(id,json,strlen(json),profile_text_apply,patch);cJSON_free(json);}else e=ESP_ERR_NO_MEM;
        }
    }free(patch);cJSON_Delete(p);return e;
}

#include "yao_service.h"
#include <string.h>
#include <stdio.h>
#ifndef BADGE_CONTROL_HOST_TEST
#include "badge_network.h"
#include "esp_random.h"
bool yao_random_bits(uint32_t *bits) {
    if(!bits||!badge_network_entropy_ready())return false;
    esp_fill_random(bits,sizeof(*bits));return true;
}
#endif
static yao_record_t cache[YAO_CACHE_COUNT];
static uint32_t sequence;
bool yao_lookup(uint32_t id,yao_record_t *out){
    if(!id||!out)return false;
    for(unsigned i=0;i<YAO_CACHE_COUNT;i++)if(cache[i].id==id){*out=cache[i];return true;}
    return false;
}
bool yao_latest(yao_record_t *out){return yao_lookup(sequence,out);}
static bool save(yao_record_t *r){
    if(sequence==UINT32_MAX)return false;
    r->cast_utc=yao_time_now();r->location=yao_location_get();
    r->id=++sequence;cache[(sequence-1)%YAO_CACHE_COUNT]=*r;return true;
}
bool yao_save_manual(const uint8_t lines[6],yao_record_t *out){
    yao_result_t result;if(!out||!yao_calculate(lines,&result))return false;
    *out=(yao_record_t){0};memcpy(out->lines,lines,6);return save(out);
}
bool yao_refresh_location(yao_record_t *record){
    if(!record||!record->id||record->location.configured)return false;
    yao_location_t location=yao_location_get();if(!location.configured||!yao_location_valid(&location))return false;
    record->location=location;
    for(unsigned i=0;i<YAO_CACHE_COUNT;i++)if(cache[i].id==record->id){cache[i].location=location;break;}
    return true;
}
static cJSON *hexagram(const yao_hexagram_t *h){
    char name[32];if(!yao_full_name(h,name,sizeof(name)))return NULL;
    cJSON *o=cJSON_CreateObject();
    if(o&&cJSON_AddNumberToObject(o,"number",h->number)&&cJSON_AddStringToObject(o,"name",name)&&cJSON_AddStringToObject(o,"short_name",h->name)&&cJSON_AddStringToObject(o,"guaci",h->text)){
        cJSON *a=cJSON_AddArrayToObject(o,"yaoci");if(!a)goto fail;
        for(unsigned i=0;i<6;i++){
            cJSON *line=cJSON_CreateString(h->lines[i]);if(!line)goto fail;
            if(!cJSON_AddItemToArray(a,line)){cJSON_Delete(line);goto fail;}
        }
        if(h->special&&!cJSON_AddStringToObject(o,"special",h->special))goto fail;
        return o;
    }
fail:
    cJSON_Delete(o);return NULL;
}
static bool attach(cJSON *o,const char *key,cJSON *value){
    if(!value)return false;
    if(cJSON_AddItemToObject(o,key,value))return true;
    cJSON_Delete(value);return false;
}
cJSON *yao_time_json(int64_t utc,const yao_location_t *location){
    cJSON *o=cJSON_CreateObject();if(!o)return NULL;
    bool valid=yao_time_valid(utc),located=location&&location->configured&&yao_location_valid(location);
    if(!cJSON_AddBoolToObject(o,"clock_valid",valid)||!cJSON_AddBoolToObject(o,"location_set",located))goto fail;
    if(located&&(!cJSON_AddStringToObject(o,"location_source","network_ip_approx")||!cJSON_AddStringToObject(o,"region",location->region)||!cJSON_AddNumberToObject(o,"longitude_east",location->longitude_e6/1000000.0)))goto fail;
    if(!valid)return o;
    char stamp[20];if(!yao_time_format(utc,stamp)||!cJSON_AddStringToObject(o,"utc",stamp))goto fail;
    if(!yao_time_format(utc+28800,stamp)||!cJSON_AddStringToObject(o,"beijing_utc8",stamp))goto fail;
    if(located){
        int64_t solar;double eq;
        if(!yao_solar_time(utc,location->longitude_e6/1000000.0,&solar,&eq)||!yao_time_format(solar,stamp))goto fail;
        if(!cJSON_AddStringToObject(o,"true_solar_time",stamp)||
           !cJSON_AddNumberToObject(o,"equation_minutes",(double)((int)(eq*100))/100)||
           !cJSON_AddStringToObject(o,"solar_method","NOAA fractional-year approximation; solar wall time, not UTC"))goto fail;
    }
    return o;
fail:cJSON_Delete(o);return NULL;
}
cJSON *yao_query_time_json(void){
    cJSON *o=yao_time_json(yao_time_now(),NULL);
    if(o)cJSON_DeleteItemFromObjectCaseSensitive(o,"location_set");
    return o;
}
cJSON *yao_record_json(const yao_record_t *record){
    yao_result_t r;if(!record||!yao_calculate(record->lines,&r))return NULL;
    cJSON *o=cJSON_CreateObject();if(!o)return NULL;
    if(!cJSON_AddNumberToObject(o,"result_id",record->id)||
       !cJSON_AddStringToObject(o,"question",record->question)||
       !cJSON_AddStringToObject(o,"line_order","bottom_to_top")||
       !cJSON_AddStringToObject(o,"random_source","esp32_rf_rng")||
       !cJSON_AddBoolToObject(o,"survives_reboot",false)||
       !attach(o,"cast_time",yao_time_json(record->cast_utc,&record->location))||
       !attach(o,"current_time",yao_query_time_json())||
       !attach(o,"original",hexagram(r.original))||!attach(o,"changed",hexagram(r.changed))||
       !cJSON_AddStringToObject(o,"rule",yao_reading_rule(r.moving_count))||
       !cJSON_AddStringToObject(o,"instruction","若question及当前对话均未明确所问事项，先询问想问哪件事及必要背景，等用户补充后再解读，不自行假定事业或感情。完整卦辞、六条爻辞及用九用六仅作依据；默认直接用白话讲结论、趋势、注意事项和建议，结合主读和动爻综合解释。不要先朗读原文或逐爻翻译；仅用户明确要求时展开。卦名用全称，无动爻不重复。起卦依据cast_time，不用current_time替换；真太阳时已按起卦所在地经度与均时差修正，勿再次修正。location_set=false时直接用已有系统时间解读，简短说明未校准真太阳时，不要求用户填写地区或经度；clock_valid=false时说明时间未知，不编造时间相关推断。不要重新起卦，不把解读说成必然事实。"))goto fail;
    cJSON *lines=cJSON_AddArrayToObject(o,"lines"),*moving=cJSON_AddArrayToObject(o,"moving_lines"),*readings=cJSON_AddArrayToObject(o,"readings");
    if(!lines||!moving||!readings)goto fail;
    for(unsigned i=0;i<6;i++){
        cJSON *n=cJSON_CreateNumber(r.lines[i]);if(!n)goto fail;cJSON_AddItemToArray(lines,n);
        if(r.moving_mask&(1u<<i)){n=cJSON_CreateNumber(i+1);if(!n)goto fail;cJSON_AddItemToArray(moving,n);}
    }
    for(unsigned i=0;i<r.reading_count;i++){
        const yao_reading_t *p=&r.readings[i];cJSON *v=cJSON_CreateObject();if(!v)goto fail;
        cJSON_AddItemToArray(readings,v);
        char name[32];if(!yao_full_name(p->hexagram,name,sizeof(name)))goto fail;
        if(!cJSON_AddStringToObject(v,"source",p->hexagram==r.original?"original":"changed")||
           !cJSON_AddStringToObject(v,"hexagram_name",name)||
           !cJSON_AddNumberToObject(v,"line",p->line)||!cJSON_AddBoolToObject(v,"primary",p->primary)||
           !cJSON_AddStringToObject(v,"text",p->text))goto fail;
    }
    return o;
fail:cJSON_Delete(o);return NULL;
}
static bool valid_text(const char *s,unsigned max_chars,unsigned max_bytes){
    if(!s||strlen(s)>max_bytes)return false;
    unsigned count=0;const unsigned char *p=(const unsigned char *)s;
    while(*p){
        unsigned cp=*p++,n=0,min=0;
        if(cp<0x80){if(cp<32||cp==127)return false;}
        else if(cp>=0xc2&&cp<=0xdf){cp&=31;n=1;min=0x80;}
        else if(cp>=0xe0&&cp<=0xef){cp&=15;n=2;min=0x800;}
        else if(cp>=0xf0&&cp<=0xf4){cp&=7;n=3;min=0x10000;}
        else return false;
        for(unsigned i=0;i<n;i++){if((*p&0xc0)!=0x80)return false;cp=(cp<<6)|(*p++&63);}
        if(cp<min||cp>0x10ffff||(cp>=0xd800&&cp<=0xdfff))return false;
        if(++count>max_chars)return false;
    }
    return true;
}
cJSON *yao_tool(const char *name,const cJSON *args,const char **error){
    *error=NULL;yao_record_t record={0};
    bool cast=!strcmp(name,"self.yao.cast");
    if(args&&!cJSON_IsObject(args))goto invalid;
    unsigned seen=0;
    for(const cJSON *v=args?args->child:NULL;v;v=v->next){
        unsigned bit=!strcmp(v->string,cast?"request_id":"result_id")?1:cast&&!strcmp(v->string,"question")?2:0;
        if(!bit||(seen&bit))goto invalid;
        seen|=bit;
    }
    if(!cast){
        const cJSON *id=cJSON_GetObjectItemCaseSensitive(args,"result_id");
        if(id&&(!cJSON_IsNumber(id)||id->valuedouble<1||id->valuedouble>UINT32_MAX||id->valuedouble!=(uint32_t)id->valuedouble))goto invalid;
        if(!(id?yao_lookup((uint32_t)id->valuedouble,&record):yao_latest(&record))){*error="没有这次卦象。仅保留本次开机最近四卦，请先起卦；不要自动重新起卦。";return NULL;}
    }else{
        const cJSON *key=cJSON_GetObjectItemCaseSensitive(args,"request_id"),*q=cJSON_GetObjectItemCaseSensitive(args,"question");
        if(!cJSON_IsString(key)||!key->valuestring[0]||strlen(key->valuestring)>48||(q&&!cJSON_IsString(q)))goto invalid;
        for(const unsigned char *p=(const unsigned char *)key->valuestring;*p;p++)if(!((*p>='0'&&*p<='9')||(*p>='A'&&*p<='Z')||(*p>='a'&&*p<='z')||*p=='-'||*p=='_'))goto invalid;
        const char *question=q?q->valuestring:"";if(!valid_text(question,96,384))goto invalid;
        for(unsigned i=0;i<YAO_CACHE_COUNT;i++)if(cache[i].id&&!strcmp(cache[i].request,key->valuestring)){
            if(strcmp(cache[i].question,question)){*error="同一 request_id 不能用于不同问题。";return NULL;}
            return yao_record_json(&cache[i]);
        }
        uint32_t bits;if(!yao_random_bits(&bits)){*error="硬件随机源尚未就绪，请稍后用相同 request_id 重试。未生成卦象。";return NULL;}
        for(unsigned i=0;i<6;i++)record.lines[i]=yao_coin_line((uint8_t)(bits>>(3*i)));
        strcpy(record.request,key->valuestring);strcpy(record.question,question);
        if(!save(&record)){*error="本次开机的结果编号已用尽。";return NULL;}
    }
    return yao_record_json(&record);
invalid:*error="参数无效，请按方法定义填写。";return NULL;
}
char *yao_interpretation_prompt(const yao_record_t *record){
    if(!record||!record->id)return NULL;
    cJSON *snapshot=yao_record_json(record);if(!snapshot)return NULL;
    char *out=cJSON_PrintUnformatted(snapshot);cJSON_Delete(snapshot);return out;
}

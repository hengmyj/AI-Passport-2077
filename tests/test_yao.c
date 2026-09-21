#include "yao_service.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static bool entropy=true;
static uint32_t random_word;
static unsigned random_calls;
bool yao_random_bits(uint32_t *out){random_calls++;if(!entropy)return false;*out=random_word;return true;}
static cJSON *call(const char *name,const char *args,bool ok){
    cJSON *a=cJSON_Parse(args);assert(a);const char *error=NULL;cJSON *r=yao_tool(name,a,&error);cJSON_Delete(a);
    assert(ok?(r&&!error):(!r&&error));return r;
}
int main(void){
    static const char *const names[]={
        "乾为天","坤为地","水雷屯","山水蒙","水天需","天水讼","地水师","水地比",
        "风天小畜","天泽履","地天泰","天地否","天火同人","火天大有","地山谦","雷地豫",
        "泽雷随","山风蛊","地泽临","风地观","火雷噬嗑","山火贲","山地剥","地雷复",
        "天雷无妄","山天大畜","山雷颐","泽风大过","坎为水","离为火","泽山咸","雷风恒",
        "天山遁","雷天大壮","火地晋","地火明夷","风火家人","火泽睽","水山蹇","雷水解",
        "山泽损","风雷益","泽天夬","天风姤","泽地萃","地风升","泽水困","水风井",
        "泽火革","火风鼎","震为雷","艮为山","风山渐","雷泽归妹","雷火丰","火山旅",
        "巽为风","兑为泽","风水涣","水泽节","风泽中孚","雷山小过","水火既济","火水未济"
    };
    for(unsigned i=0;i<64;i++){char name[32];assert(yao_full_name(&YAO_HEXAGRAMS[i],name,sizeof(name)));assert(!strcmp(name,names[i]));}
    unsigned distribution[4]={0};
    for(unsigned i=0;i<8;i++)distribution[yao_coin_line(i)-6]++;
    assert(distribution[0]==1&&distribution[1]==3&&distribution[2]==3&&distribution[3]==1);
    bool originals[64]={0};unsigned max_json=0,max_reading=0;
    for(unsigned pattern=0;pattern<4096;pattern++){
        uint8_t lines[6],changed[6];unsigned moving=0;
        for(unsigned i=0;i<6;i++){lines[i]=6+((pattern>>(2*i))&3);changed[i]=lines[i]==6?7:lines[i]==9?8:lines[i];moving+=lines[i]==6||lines[i]==9;}
        yao_result_t r,c;assert(yao_calculate(lines,&r)&&yao_calculate(changed,&c));
        assert(r.moving_count==moving&&r.changed==c.original);originals[r.original->number-1]=true;
        assert(r.reading_count==((moving==2||moving==3||moving==4)?2:1));
        assert(r.readings[0].primary&&r.readings[0].text&&*r.readings[0].text);
        if(r.reading_count==2)assert(!r.readings[1].primary);
        if(moving==2)assert(r.readings[0].line>r.readings[1].line);
        if(moving==4)assert(r.readings[0].line<r.readings[1].line);
        yao_record_t record={.id=1,.cast_utc=1800000000,.location={.region="起卦测试地区",.longitude_e6=116407400,.configured=true}};memcpy(record.lines,lines,6);
        /* Largest accepted UTF-8 question and JSON-escaped region stay bounded. */
        for(unsigned i=0;i<96;i++)memcpy(record.question+4*i,"\xF0\x9F\x98\x80",4);
        memset(record.location.region,'"',63);record.location.region[63]=0;
        cJSON *o=yao_record_json(&record);assert(o);char *s=cJSON_PrintUnformatted(o);assert(s);
        unsigned len=(unsigned)strlen(s);if(len>max_json)max_json=len;
        for(unsigned side=0;side<2;side++){
            const yao_hexagram_t *h=side?r.changed:r.original;
            cJSON *hex=cJSON_GetObjectItemCaseSensitive(o,side?"changed":"original");
            assert(!strcmp(cJSON_GetObjectItemCaseSensitive(hex,"name")->valuestring,names[h->number-1]));
            assert(!strcmp(cJSON_GetObjectItemCaseSensitive(hex,"guaci")->valuestring,h->text));
            cJSON *yaoci=cJSON_GetObjectItemCaseSensitive(hex,"yaoci");assert(cJSON_GetArraySize(yaoci)==6);
            for(unsigned i=0;i<6;i++)assert(!strcmp(cJSON_GetArrayItem(yaoci,i)->valuestring,h->lines[i]));
            if(h->special)assert(!strcmp(cJSON_GetObjectItemCaseSensitive(hex,"special")->valuestring,h->special));
        }
        char full[4096],small[8];size_t needed=yao_format_full_reading(&r,NULL,0);
        assert(needed<sizeof(full));assert(yao_format_full_reading(&r,full,sizeof(full))==needed&&strlen(full)==needed);
        assert(yao_format_full_reading(&r,small,sizeof(small))==needed&&small[7]==0);
        if(needed>max_reading)max_reading=(unsigned)needed;
        for(unsigned i=0;i<6;i++){
            assert(strstr(full,r.original->lines[i]));if(moving)assert(strstr(full,r.changed->lines[i]));
        }
        assert(strstr(full,r.original->text));if(moving)assert(strstr(full,r.changed->text));
        assert(moving||!strstr(full,"之卦 · "));
        if(r.original->special)assert(strstr(full,r.original->special));

        assert(len<4096-500&&!strstr(s,"%s"));cJSON_free(s);cJSON_Delete(o);
    }
    for(unsigned i=0;i<64;i++)assert(originals[i]&&YAO_HEXAGRAMS[i].number==i+1);
    uint8_t qian[]={9,9,9,9,9,9},kun[]={6,6,6,6,6,6},ji[]={7,8,7,8,7,8};yao_result_t r;
    assert(yao_calculate(qian,&r)&&r.original->number==1&&r.changed->number==2&&r.readings[0].line==7&&strstr(r.readings[0].text,"用九"));
    assert(yao_calculate(kun,&r)&&r.original->number==2&&r.changed->number==1&&strstr(r.readings[0].text,"用六"));
    assert(yao_calculate(ji,&r)&&r.original->number==63&&r.moving_count==0);
    ji[0]=10;assert(!yao_calculate(ji,&r));
    call("self.yao.get","{}",false);
    entropy=false;call("self.yao.cast","{\"request_id\":\"a\",\"question\":\"工作\"}",false);
    entropy=true;random_word=0;
    cJSON *o=call("self.yao.cast","{\"request_id\":\"a\",\"question\":\"工作\"}",true);char *first=cJSON_PrintUnformatted(o);cJSON_Delete(o);
    unsigned draws=random_calls;random_word=UINT32_MAX;entropy=false;
    o=call("self.yao.cast","{\"request_id\":\"a\",\"question\":\"工作\"}",true);char *again=cJSON_PrintUnformatted(o);assert(!strcmp(first,again)&&random_calls==draws);cJSON_free(first);cJSON_free(again);cJSON_Delete(o);
    call("self.yao.cast","{\"request_id\":\"a\",\"question\":\"不同\"}",false);
    o=call("self.yao.get","{}",true);cJSON_Delete(o);assert(random_calls==draws);
    const char *bad[]={"{}","[]","{\"request_id\":1}","{\"request_id\":\"\"}","{\"request_id\":\"a!\"}","{\"request_id\":\"a\",\"request_id\":\"a\"}","{\"request_id\":\"a\",\"question\":null}","{\"request_id\":\"a\",\"question\":\"a\\nb\"}"};
    for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);i++)call("self.yao.cast",bad[i],false);
    call("self.yao.get","{\"result_id\":0}",false);call("self.yao.get","{\"result_id\":1.5}",false);call("self.yao.get","{\"extra\":1}",false);
    yao_record_t record;assert(yao_save_manual(qian,&record)&&record.id==2);assert(yao_latest(&record)&&record.lines[0]==9);
    char *prompt=yao_interpretation_prompt(&record);assert(prompt&&strstr(prompt,"\"result_id\":2")&&strstr(prompt,"用九")&&strlen(prompt)<4096);cJSON_free(prompt);
    /* A query must retain the cast's instant and location after time/settings change. */
    yao_location_t place={.region="Beijing",.longitude_e6=116407400,.configured=true};
    yao_location_network(false);yao_location_network(true);yao_location_finish(yao_location_begin(),&place);yao_test_now=1735689600;
    assert(yao_refresh_location(&record)&&record.location.configured);yao_record_t refreshed;assert(yao_lookup(record.id,&refreshed)&&refreshed.location.configured);
    assert(yao_save_manual(qian,&record));
    o=yao_record_json(&record);assert(o);
    assert(!strcmp(cJSON_GetObjectItemCaseSensitive(o,"question")->valuestring,""));
    assert(strstr(cJSON_GetObjectItemCaseSensitive(o,"instruction")->valuestring,"等用户补充后再解读"));
    char *cast=cJSON_PrintUnformatted(cJSON_GetObjectItemCaseSensitive(o,"cast_time"));cJSON_Delete(o);assert(cast);
    /* A new cast uses live UTC with the same cached longitude. */
    yao_test_now+=60;yao_record_t later;
    assert(yao_save_manual(kun,&later));
    assert(later.cast_utc==record.cast_utc+60&&later.location.longitude_e6==record.location.longitude_e6);
    int64_t solar_before,solar_after;double eq;
    assert(yao_solar_time(record.cast_utc,record.location.longitude_e6/1000000.0,&solar_before,&eq));
    assert(yao_solar_time(later.cast_utc,later.location.longitude_e6/1000000.0,&solar_after,&eq));
    assert(solar_after-solar_before>=59&&solar_after-solar_before<=61);
    yao_test_now+=3600;yao_location_test_ms+=3600000;place.longitude_e6=121000000;strcpy(place.region,"Changed");yao_location_network(false);yao_location_network(true);yao_location_finish(yao_location_begin(),&place);
    assert(yao_lookup(record.id,&record));o=yao_record_json(&record);
    char *retained=cJSON_PrintUnformatted(cJSON_GetObjectItemCaseSensitive(o,"cast_time"));assert(retained&&!strcmp(cast,retained));
    assert(strcmp(cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(o,"current_time"),"utc")->valuestring,"2025-01-01 00:00:00"));
    cJSON_free(cast);cJSON_free(retained);cJSON_Delete(o);
    yao_test_now=0;assert(yao_save_manual(kun,&record));o=yao_record_json(&record);
    cJSON *clock=cJSON_GetObjectItemCaseSensitive(o,"cast_time");assert(cJSON_IsFalse(cJSON_GetObjectItemCaseSensitive(clock,"clock_valid"))&&!cJSON_HasObjectItem(clock,"true_solar_time"));cJSON_Delete(o);
    for(unsigned i=0;i<4;i++)assert(yao_save_manual(kun,&record));
    assert(!yao_lookup(1,&record)&&yao_latest(&record));
    printf("Yao: all 4096 outcomes, 64 hexagrams, 7 reading rules, replay/failure/history PASS; max JSON=%u; max full reading=%u\n",max_json,max_reading);
}

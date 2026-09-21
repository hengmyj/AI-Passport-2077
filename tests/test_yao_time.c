#include "yao_time.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
int main(void){
    int64_t solar,west,east;double eq,e2;char text[20];
    /* 2025-01-01 00:00 UTC: negative equation of time, UTC day/year rollover. */
    assert(yao_solar_time(1735689600,0,&solar,&eq));
    assert(eq> -3.0&&eq< -2.5&&solar<1735689600);
    assert(yao_time_format(solar,text)&&!strncmp(text,"2024-12-31 23:57:",16));
    assert(yao_solar_time(1735689600,120,&east,&e2)&&east-solar==28800);
    assert(yao_solar_time(1735689600,-120,&west,&e2)&&solar-west==28800);
    assert(yao_solar_time(1735689600,180,&east,&e2));
    assert(yao_solar_time(1735689600,-180,&west,&e2)&&east-west==86400);
    /* Leap day: absolute seconds carry correctly across the calendar boundary. */
    assert(yao_solar_time(1709250600,120,&solar,&eq));
    assert(yao_time_format(solar,text)&&!strncmp(text,"2024-03-01",10));
    assert(!yao_solar_time(0,120,&solar,&eq));
    assert(!yao_solar_time(1735689600,NAN,&solar,&eq));
    assert(!yao_solar_time(1735689600,181,&solar,&eq));
    assert(!yao_solar_time(1735689600,-181,&solar,&eq));
    /* Around February/November the correction changes sign, not a fixed offset. */
    assert(yao_solar_time(1739361600,0,&solar,&eq)&&eq< -14&&eq> -15);
    assert(yao_solar_time(1762257600,0,&solar,&eq)&&eq>16&&eq<17);
    yao_location_t p={0};assert(yao_location_valid(&p));
    p.configured=true;assert(!yao_location_valid(&p));
    strcpy(p.region,"Greenwich");assert(yao_location_valid(&p)); /* zero longitude is real */
    strcpy(p.region," \n");assert(!yao_location_valid(&p));
    memset(p.region,'x',sizeof(p.region));assert(!yao_location_valid(&p));
    char summary[320];
    p=(yao_location_t){.region="Greenwich",.longitude_e6=0,.configured=true};
    yao_time_summary(1735689600,&p,summary,sizeof(summary));
    assert(strstr(summary,"真太阳时（近似）")&&strstr(summary,"2024-12-31 23:57:")&&strstr(summary,"2025-01-01 08:00:00"));
    yao_time_summary(1735689600,NULL,summary,sizeof(summary));
    assert(strstr(summary,"真太阳时未校准")&&strstr(summary,"2025-01-01 08:00:00"));
    yao_time_summary(0,&p,summary,sizeof(summary));assert(strstr(summary,"起卦时未校时"));
    char tiny[2]={1,1};yao_time_summary(1735689600,&p,tiny,sizeof(tiny));assert(tiny[1]==0);
    puts("Solar time PASS: signs, UTC offset, year/leap-day rollover, missing clock/location and invalid input");
}

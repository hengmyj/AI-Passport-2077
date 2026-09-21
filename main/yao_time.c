#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include "yao_time.h"
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

bool yao_time_valid(int64_t utc){return utc>=1704067200LL&&utc<4102444800LL;}
static bool calendar(int64_t seconds,struct tm *out){
    time_t t=(time_t)seconds;
#ifdef _WIN32
    return gmtime_s(out,&t)==0;
#else
    return gmtime_r(&t,out)!=NULL;
#endif
}
bool yao_time_format(int64_t seconds,char out[20]){
    struct tm tm;
    if(!calendar(seconds,&tm)){out[0]=0;return false;}
    return strftime(out,20,"%Y-%m-%d %H:%M:%S",&tm)==19;
}
bool yao_solar_time(int64_t utc,double longitude,int64_t *solar,double *equation_minutes){
    if(!solar||!equation_minutes||!yao_time_valid(utc)||!isfinite(longitude)||longitude< -180||longitude>180)return false;
    struct tm tm;if(!calendar(utc,&tm))return false;
    const double pi=3.14159265358979323846;
    int year=tm.tm_year+1900,days=(year%4==0&&(year%100!=0||year%400==0))?366:365;
    /* NOAA General Solar Position Calculations, fractional-year approximation.
     * https://gml.noaa.gov/grad/solcalc/solareqns.PDF
     * Using UTC directly removes civil timezone/DST from the correction. */
    double hour=tm.tm_hour+tm.tm_min/60.0+tm.tm_sec/3600.0;
    double gamma=2*pi/days*(tm.tm_yday+(hour-12)/24);
    double eq=229.18*(0.000075+0.001868*cos(gamma)-0.032077*sin(gamma)
                     -0.014615*cos(2*gamma)-0.040849*sin(2*gamma));
    *equation_minutes=eq;
    *solar=utc+(int64_t)llround(240*longitude+60*eq);
    return true;
}
bool yao_location_valid(const yao_location_t *p){
    if(!p||p->longitude_e6< -180000000||p->longitude_e6>180000000||!memchr(p->region,0,sizeof(p->region)))return false;
    if(!p->configured)return !p->region[0]&&p->longitude_e6==0;
    bool text=false;const unsigned char *s=(const unsigned char *)p->region;
    for(;*s;s++){if(*s<32||*s==127)return false;if(*s!=' ')text=true;}
    return text;
}
void yao_time_summary(int64_t utc,const yao_location_t *location,char *out,size_t size){
    if(!out||!size)return;
    char civil[20],solar_text[20];int64_t solar;double equation;
    if(!yao_time_valid(utc)||!yao_time_format(utc+8*3600,civil)){
        snprintf(out,size,"起卦时未校时\n真太阳时不可用\n\n");return;
    }
    if(location&&location->configured&&yao_location_valid(location)&&
       yao_solar_time(utc,location->longitude_e6/1000000.0,&solar,&equation)&&yao_time_format(solar,solar_text)){
        snprintf(out,size,"真太阳时（近似）\n%s\n起卦时间（北京时间）\n%s\n网络定位：%s\n\n",solar_text,civil,location->region);
    }else{
        snprintf(out,size,"真太阳时未校准\n起卦时间（北京时间）\n%s\n起卦时无可用定位\n\n",civil);
    }
}
#ifdef BADGE_CONTROL_HOST_TEST
int64_t yao_test_now=1800000000;
int64_t yao_time_now(void){return yao_test_now;}
#else
int64_t yao_time_now(void){return (int64_t)time(NULL);}
#endif

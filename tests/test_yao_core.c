#include "yao_core.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
    unsigned distribution[4]={0};
    for(unsigned i=0;i<8;i++)distribution[yao_coin_line(i)-6]++;
    assert(distribution[0]==1&&distribution[1]==3&&distribution[2]==3&&distribution[3]==1);
    bool originals[64]={0};
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

    }
    for(unsigned i=0;i<64;i++)assert(originals[i]&&YAO_HEXAGRAMS[i].number==i+1);
    uint8_t qian[]={9,9,9,9,9,9},kun[]={6,6,6,6,6,6},ji[]={7,8,7,8,7,8};yao_result_t r;
    assert(yao_calculate(qian,&r)&&r.original->number==1&&r.changed->number==2&&r.readings[0].line==7&&strstr(r.readings[0].text,"用九"));
    assert(yao_calculate(kun,&r)&&r.original->number==2&&r.changed->number==1&&strstr(r.readings[0].text,"用六"));
    assert(yao_calculate(ji,&r)&&r.original->number==63&&r.moving_count==0);
    ji[0]=10;assert(!yao_calculate(ji,&r));
    puts("Yao core: 4096 outcomes / 64 hexagrams PASS");
}

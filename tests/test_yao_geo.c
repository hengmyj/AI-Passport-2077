#include "yao_geo.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 yao_location_t p;
 assert(yao_location_parse("{\"success\":true,\"city\":\"Test\",\"longitude\":0}",&p)&&p.configured&&p.longitude_e6==0);
 assert(yao_location_parse("{\"success\":true,\"region\":\"West\",\"longitude\":-74.006}",&p)&&p.longitude_e6==-74006000);
 assert(yao_location_parse("{\"status\":\"success\",\"regionName\":\"Backup\",\"city\":\"Fallback\",\"lon\":121.4737}",&p)&&p.configured&&p.longitude_e6==121473700&&!strcmp(p.region,"Fallback"));
 const char *bad[]={"{}","bad","{\"success\":false,\"city\":\"Test\",\"longitude\":120}","{\"success\":true,\"city\":\"Test\"}","{\"success\":true,\"city\":\"Test\",\"longitude\":181}","{\"success\":true,\"city\":\"Test\",\"longitude\":\"120\"}","{\"success\":true,\"city\":\"\",\"longitude\":120}"};
 for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);i++)assert(!yao_location_parse(bad[i],&p)&&!p.configured);
 puts("IP location parser PASS: primary/fallback schemas, zero/west longitude, unavailable API, absent/invalid fields");
}

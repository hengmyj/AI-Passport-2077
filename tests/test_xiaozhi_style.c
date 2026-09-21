#include "xiaozhi_style.h"
#include "nvs.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int failure,exists,opens;static uint8_t persisted,pending;
esp_err_t nvs_open(const char *name,int mode,nvs_handle_t *h){assert(!strcmp(name,"xiaozhi"));(void)mode;opens++;*h=1;return failure==1?-1:0;}
esp_err_t nvs_get_u8(nvs_handle_t h,const char *key,uint8_t *v){assert(h==1&&!strcmp(key,"face_style"));*v=persisted;return exists?0:-1;}
esp_err_t nvs_set_u8(nvs_handle_t h,const char *key,uint8_t v){assert(h==1&&!strcmp(key,"face_style"));pending=v;return failure==2?-1:0;}
esp_err_t nvs_commit(nvs_handle_t h){assert(h==1);if(failure==3)return -1;exists=1;persisted=pending;return 0;}
void nvs_close(nvs_handle_t h){assert(h==1);}
int main(void){
    xiaozhi_style_init();assert(xiaozhi_style_get()==XZ_STYLE_CUTE);
    assert(xiaozhi_style_set(XZ_STYLE_ABSTRACT));xiaozhi_style_init();assert(xiaozhi_style_get()==XZ_STYLE_ABSTRACT);
    int count=opens;for(int i=0;i<100;i++)assert(xiaozhi_style_get()==XZ_STYLE_ABSTRACT);assert(opens==count);
    assert(!xiaozhi_style_set(99)&&opens==count);
    for(failure=1;failure<=3;failure++){assert(!xiaozhi_style_set(XZ_STYLE_CUTE));assert(xiaozhi_style_get()==XZ_STYLE_ABSTRACT);}
    failure=0;xiaozhi_style_init();assert(xiaozhi_style_get()==XZ_STYLE_ABSTRACT);
    assert(xiaozhi_style_set(XZ_STYLE_CUTE));xiaozhi_style_init();assert(xiaozhi_style_get()==XZ_STYLE_CUTE);
    assert(xiaozhi_style_set(XZ_STYLE_FEMALE));xiaozhi_style_init();assert(xiaozhi_style_get()==XZ_STYLE_FEMALE);
    assert(xiaozhi_style_set(XZ_STYLE_ROUND));xiaozhi_style_init();assert(xiaozhi_style_get()==XZ_STYLE_ROUND);
    persisted=255;xiaozhi_style_init();assert(xiaozhi_style_get()==XZ_STYLE_CUTE);
    puts("Face style: persisted switch, cached reads, invalid values and NVS failure recovery PASS");
}

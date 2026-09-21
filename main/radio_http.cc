#include "radio_http.h"
bool radio_http_open(esp_http_client_handle_t client,unsigned redirects){
    for(unsigned i=0;i<=redirects;i++){
        if(esp_http_client_open(client,0)!=ESP_OK)return false;
        if(esp_http_client_fetch_headers(client)<0)return false;
        const int status=esp_http_client_get_status_code(client);
        if(status>=200&&status<300)return true;
        if(i==redirects||!(status==301||status==302||status==303||status==307||status==308))return false;
        if(esp_http_client_set_redirection(client)!=ESP_OK)return false;
        esp_http_client_close(client);
    }
    return false;
}

#pragma once
#include <stdbool.h>
#include <string.h>
/* MCP discovery belongs to the transport, not the audio conversation. The
 * service sends initialize BEFORE hello on a fresh MQTT connection. Only
 * read-only setup is exempt; commands and speech still require the live session. */
static inline bool xz_message_allowed(const char *type,const char *method,
                                     bool hello,bool has_session,bool matches){
    if(!strcmp(type,"hello"))return true;
    if(!strcmp(type,"mcp")&&(!strcmp(method,"initialize")||!strcmp(method,"tools/list")||
                            !strcmp(method,"notifications/initialized")))return true;
    return hello&&(!has_session||matches);
}

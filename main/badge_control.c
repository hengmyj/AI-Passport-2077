#include "badge_control.h"
#include "voice_catalog.h"
#include <stddef.h>
#include <stdio.h>
#include "radio_tool_catalog.h"
#ifndef BADGE_CONTROL_HOST_TEST
#include "freertos/FreeRTOS.h"
static portMUX_TYPE mux=portMUX_INITIALIZER_UNLOCKED;
#define LOCK() portENTER_CRITICAL(&mux)
#define UNLOCK() portEXIT_CRITICAL(&mux)
#else
/* Deterministic single-threaded host tests exercise the same mailbox logic. */
#define LOCK() ((void)0)
#define UNLOCK() ((void)0)
#endif
static badge_control_status_t state;
static badge_control_command_t pending;
static uint32_t serial;
static uint32_t created_id;
void badge_control_set_created_id(uint32_t id){LOCK();created_id=id;UNLOCK();}
uint32_t badge_control_created_id(void){LOCK();uint32_t id=created_id;UNLOCK();return id;}
static unsigned phase;
static void (*wakeup)(void);
void badge_control_set_wakeup(void (*wake)(void)){LOCK();wakeup=wake;UNLOCK();}
bool badge_control_stays(badge_control_kind_t k){return k==BC_BRIGHTNESS||k==BC_XZ_VOLUME||k==BC_REMINDER_SET||k==BC_REMINDER_CANCEL||k==BC_PROFILE_EDIT;}
bool badge_control_peek(uint32_t ticket,badge_control_command_t *c){LOCK();bool valid=ticket&&ticket==serial&&phase==1;if(valid)*c=pending;UNLOCK();return valid;}
int badge_control_result(uint32_t ticket){LOCK();int result=!ticket||ticket!=serial?-2:phase?0:state.last_result;UNLOCK();return result;}
void badge_control_publish(unsigned brightness,unsigned mask,unsigned qr_mask,unsigned count,unsigned active){
    LOCK();state.brightness=brightness;state.mask=mask;state.qr_mask=qr_mask;state.count=count;state.active=active;UNLOCK();
}
badge_control_status_t badge_control_status(void){LOCK();badge_control_status_t copy=state;UNLOCK();return copy;}
void badge_control_identity(unsigned slot,const char *name){if(slot>=5)return;LOCK();snprintf(state.names[slot],129,"%s",name);UNLOCK();}
void badge_control_runtime(int battery,bool active,unsigned remaining,const char *text){LOCK();state.battery=battery;state.reminder_active=active;state.reminder_seconds=remaining;snprintf(state.reminder_text,97,"%s",text);UNLOCK();}
void badge_control_enable(bool enabled){LOCK();state.enabled=enabled;phase=0;++serial;UNLOCK();}
void badge_control_cancel(void){LOCK();phase=0;++serial;UNLOCK();}
const char *badge_control_reserve(badge_control_command_t c,uint32_t *ticket){
    const char *error=NULL;*ticket=0;LOCK();
    if(!state.enabled)error="Xiaozhi is not active";
    else if(phase)error="Another device action is pending; do not retry until it completes";
    else if(c.kind<BC_MUYU||c.kind>BC_SHUTDOWN)error="Unsupported action";
    else if(c.kind==BC_PLAY_VOICE&&c.value>=VOICE_CLIP_COUNT)error="Unknown audio clip";
    else if(c.kind==BC_BADGE&&(c.value>=state.count||c.value>=5||!(state.mask&(1u<<c.value))))error="This badge has not been configured";
    else if(c.kind==BC_QR&&!(state.qr_mask&(1u<<state.active)))error="The active badge has no QR code; upload one in the configuration page";
    else if(c.kind==BC_BRIGHTNESS&&(c.value<20||c.value>100||c.value%20))error="Brightness must be 20, 40, 60, 80 or 100";
    else if(c.kind==BC_XZ_VOLUME&&c.value>100)error="Volume must be 0..100";
    else if(c.kind==BC_RADIO_PRESET&&c.value>=radio_tool_count())error="Unknown station";
    else if(c.kind==BC_REMINDER_SET&&(c.when<1704067200LL||c.repeat>127||c.minute>=1440||!c.text[0]))error="Invalid reminder";
    else if(c.kind==BC_REMINDER_CANCEL&&!c.value)error="A reminder ID is required";
    else if(c.kind==BC_PROFILE_EDIT&&(c.value>=state.count||c.field>2||!(state.mask&(1u<<c.value))))error="Unknown badge or field";
    if(!error){pending=c;phase=1;state.last_result=0;created_id=0;if(++serial==0)++serial;*ticket=serial;}
    UNLOCK();return error;
}
void badge_control_commit(uint32_t ticket,bool sent){void (*wake)(void)=NULL;LOCK();if(ticket&&ticket==serial&&phase==1){phase=sent?2:0;if(sent)wake=wakeup;else{state.last_result=-1;}}UNLOCK();if(wake)wake();}
bool badge_control_take(badge_control_command_t *c){LOCK();bool ready=state.enabled&&phase==2;if(ready){*c=pending;phase=3;}UNLOCK();return ready;}
void badge_control_finish(bool success){LOCK();phase=0;state.last_result=success?1:-1;UNLOCK();}

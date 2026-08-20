#include "Servo2Control.h"

#ifdef SERVO_PIN2
static QueueHandle_t q2;
static TaskHandle_t t2;
static volatile bool moving2;
static volatile bool stop2;
static int chan2 = -1;
static float cur2 = SCTL_RESET_ANGLE;
static float tp2;
static float pulse(float a){return SCTL_MIN_PULSE_US+(a/180.0f)*(SCTL_MAX_PULSE_US-SCTL_MIN_PULSE_US);}
static uint32_t duty(float us){return uint32_t(us/1000000.0f/tp2);}
static void out2(float a){if(chan2>=0)ledcWrite(chan2,duty(pulse(a)));}
static void task2(void*){servo_cmd_t c;for(;;)if(xQueueReceive(q2,&c,portMAX_DELAY)){moving2=true;float a=constrain(c.angle,0.0f,180.0f);if(c.speed_dps<=0)out2(a);else{float step=c.speed_dps*SCTL_STEP_INTERVAL_MS/1000.0f;if(step<0.5f)step=0.5f;int dir=a>cur2?1:-1;while((dir>0&&cur2<a)||(dir<0&&cur2>a)){if(stop2){stop2=false;break;}cur2+=dir*step;if((dir>0&&cur2>a)||(dir<0&&cur2<a))cur2=a;out2(cur2);vTaskDelay(pdMS_TO_TICKS(SCTL_STEP_INTERVAL_MS));}}cur2=a;moving2=false;}}
void servo2_init(){chan2=sys_get_next_PWM_chan_num();if(chan2<0)return;tp2=(1.0f/SCTL_PULSE_FREQ)/(1<<SCTL_PULSE_RES_BITS);ledcSetup(chan2,SCTL_PULSE_FREQ,SCTL_PULSE_RES_BITS);ledcAttachPin(SERVO_PIN2,chan2);out2(SCTL_RESET_ANGLE);q2=xQueueCreate(SCTL_QUEUE_LEN,sizeof(servo_cmd_t));if(q2&&xTaskCreatePinnedToCore(task2,"servo2_task",2048,NULL,1,&t2,tskNO_AFFINITY)==pdPASS){} }
void servo2_set_angle(float a,float f){if(!q2)return;protocol_buffer_synchronize();servo_cmd_t c{a,f};moving2=true;xQueueSend(q2,&c,pdMS_TO_TICKS(10000));uint32_t s=millis();while(moving2&&millis()-s<120000){esp_task_wdt_reset();vTaskDelay(pdMS_TO_TICKS(10));}}
void servo2_reset(){servo2_set_angle(SCTL_RESET_ANGLE,0);} void servo2_stop(){stop2=true;if(q2)xQueueReset(q2);out2(cur2);moving2=false;}
#else
void servo2_init(){} void servo2_set_angle(float,float){} void servo2_reset(){} void servo2_stop(){}
#endif

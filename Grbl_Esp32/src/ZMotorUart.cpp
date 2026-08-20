#include "ZMotorUart.h"
#include "ZMotorConfig.h"
#include "Grbl.h"
#include <algorithm>

namespace { HardwareSerial z_motor_serial(Z_MOTOR_UART); bool initialized = false; constexpr uint8_t ADDR=1;
void u16(uint8_t*p,uint16_t v){p[0]=v>>8;p[1]=v;} void u32(uint8_t*p,uint32_t v){p[0]=v>>24;p[1]=v>>16;p[2]=v>>8;p[3]=v;}
bool read_frame(uint8_t fn,uint8_t*payload,size_t n,uint32_t timeout){uint8_t f[16]={};size_t c=0;uint32_t s=millis();while(millis()-s<timeout){if(!ZMotorUart::available()){delay(1);continue;}int v=ZMotorUart::read();if(v<0)continue;if(c<sizeof(f))f[c++]=uint8_t(v);if(c>=3&&f[c-1]==0x6B){if(f[0]!=ADDR||f[1]!=fn||c<n+3)return false;if(payload&&n)memcpy(payload,&f[2],n);return f[2]==2||f[2]==0x9F;}}return false;}
bool command(const uint8_t*body,size_t n,uint8_t fn,uint32_t timeout=1000){uint8_t f[16]={};if(n+2>sizeof(f))return false;f[0]=ADDR;memcpy(&f[1],body,n);f[n+1]=0x6B;ZMotorUart::flush_input();if(ZMotorUart::write(f,n+2)!=n+2)return false;return read_frame(fn,nullptr,0,timeout);}}

namespace ZMotorUart { void init(){if(initialized)return;z_motor_serial.begin(Z_MOTOR_UART_BAUD,SERIAL_8N1,Z_MOTOR_UART_RX,Z_MOTOR_UART_TX);initialized=true;grbl_msg_sendf(CLIENT_SERIAL,MsgLevel::Info,"Z motor UART%d TX:%d RX:%d baud:%d",Z_MOTOR_UART,Z_MOTOR_UART_TX,Z_MOTOR_UART_RX,Z_MOTOR_UART_BAUD);} bool available(){return initialized&&z_motor_serial.available()>0;} int read(){return initialized?z_motor_serial.read():-1;} size_t write(const uint8_t*d,size_t n){return initialized&&d&&n?z_motor_serial.write(d,n):0;} void flush_input(){while(available())z_motor_serial.read();} }

namespace ZMotor {
bool enable(bool on){const uint8_t b[]={0xF3,0xAB,uint8_t(on),0};return command(b,sizeof(b),0xF3);}
bool stop(){const uint8_t b[]={0xFE,0x98,0};return command(b,sizeof(b),0xFE);}
bool move_pulses(int32_t pulses,uint16_t rpm,uint8_t acc,uint32_t timeout){if(!pulses)return true;uint8_t b[11]={0xFD,uint8_t(pulses<0),0,0,0,0,0,0,0,0,0};u16(&b[2],std::min<uint16_t>(rpm,3000));b[4]=acc;u32(&b[5],uint32_t(pulses<0?-int64_t(pulses):pulses));if(!command(b,sizeof(b),0xFD))return false;uint32_t s=millis();while(millis()-s<timeout){protocol_execute_realtime();if(sys.abort){stop();return false;}const uint8_t q[]={ADDR,0x3A,0x6B};uint8_t status=0;ZMotorUart::flush_input();if(ZMotorUart::write(q,sizeof(q))!=sizeof(q))return false;if(read_frame(0x3A,&status,1,100)){if(status&0x80)return false;if(status&2)return true;}delay(2);}return false;}
bool home(uint8_t mode,uint32_t timeout){const uint8_t b[]={0x9A,uint8_t(mode%6),0};return timeout&&command(b,sizeof(b),0x9A);}
bool has_alarm(){const uint8_t q[]={ADDR,0x3A,0x6B};uint8_t status=0;ZMotorUart::flush_input();if(ZMotorUart::write(q,sizeof(q))!=sizeof(q))return true;return !read_frame(0x3A,&status,1,100)||(status&0x80);}
}

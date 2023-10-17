#include "mbed.h"
//#include "hcsr04.h" //超音波センサ
#include "Dht11.h"  
#include"Air_Quality.h"
#include "MPL3115A2.h"//大気圧
#include"HEPTA_EPS.h"
#include"HEPTA_SENSOR.h"
#include"HEPTA_COM.h"
#include"HEPTA_CDH.h"

HEPTA_CDH cdh(p5, p6, p7, p8, "sd");
RawSerial pc(USBTX, USBRX); 
//HCSR04  usensor(p21,p12);//超音波センサ p21:trig p12:echo

DigitalOut led0(LED1), led25(LED2), led50(LED3), led75(LED4);//光センサ
AnalogIn temt6000(p15);//光センサ
I2C i2c(p28, p27);       // sda, scl

//MPL3115A2 psensor(&i2c, &pc);//気圧
 
DigitalOut myled(LED1);     // Sanity check to make sure the program is working.
DigitalOut powerPin(p21);   // <-- I powered the sensor from a pin. You don't have to.
//AnalogIn sensorUV(p15);//紫外線

HEPTA_EPS eps(p16,p26);

HEPTA_SENSOR sensor(p17,
                  p28,p27,0x19,0x69,0x13,
                  p13, p14,p25,p24);

HEPTA_COM com(p9,p10,9600);
 


Timer sattime;
int main() {
    char c;
    
    float UVvalue;
    int dist;
    float x = temt6000;
    float temp;
    
    int i = 0,rcmd=0,cmdflag=0;
    float bt,ax,ay,az;
    FILE *fp = fopen("/sd/mydir/test.txt","w");
    if(fp == NULL) {
        error("Could not open file for write\r\n");
        com.printf("not find sdcards/r/n");
    }

    sattime.start();
    
        
    wait(0.5);
    
    while(1){
        com.xbee_receive(&rcmd,&cmdflag);
        wait(0.5);
        sensor.temp_sense(&temp);
        wait(0.5);
        
        sensor.sen_acc(&ax,&ay,&az);
        
        wait(0.5);
        eps.vol(&bt);
        wait(0.5);

        
        sensor.gps_setting();
        int quality=0,stnum=0,gps_check=0;
        char ns='A',ew='B',aunit='m';
        float time=0.0,latitude=0.0,longitude=0.0,hacu=0.0,altitude=0.0;
        int flag=0;
        if (flag == 0) {
            sensor.gga_sensing(&time, &latitude, &ns, &longitude, &ew, &quality, &stnum, &hacu, &altitude, &aunit, &gps_check);
            if((gps_check==0)|(gps_check==1)) {
                //pc.printf("latitude: %f  longitude: %f\r\n",latitude,longitude);
                flag=1;
            }
        }

        com.printf("sattime:%f  V: %f  T: %f acc : %f,%f,%f",sattime.read(),bt,temp,ax,ay,az);
        wait(1);
        com.printf("lat: %f  lon: %f \r\n",latitude,longitude);
        


        //com.printf("light sensor : %f\r\n",x);
        wait(1);

        if (cmdflag == 1) {
            if (rcmd == 'a') {
                com.printf("get a\r\n");
                wait(0.5);
            }
            com.initialize();
            break;
        }
    }

    //get sensor data lite-sensor-attitude
    while(1){
        com.xbee_receive(&rcmd,&cmdflag);
        
        wait(0.5);
        sensor.temp_sense(&temp);
        
        wait(0.5);
        eps.vol(&bt);
        wait(0.5);
        sensor.sen_acc(&ax,&ay,&az);
        wait(0.5);
      

        sensor.gps_setting();

        int quality=0,stnum=0,gps_check=0;
        char ns='A',ew='B',aunit='m';
        float time=0.0,latitude=0.0,longitude=0.0,hacu=0.0,altitude=0.0;
        int flag=0;
        if (flag == 0) {
            sensor.gga_sensing(&time, &latitude, &ns, &longitude, &ew, &quality, &stnum, &hacu, &altitude, &aunit, &gps_check);
            if((gps_check==0)|(gps_check==1)) {
                //pc.printf("latitude: %f  longitude: %f\r\n",latitude,longitude);
                flag=1;
            }
        }

        float x = temt6000;
    
        // four LEDs meaning "light level" (1 to 4):
        led0 = 1;
        led25 = led50 = led75 = 0;
        if(x>0.25) led25 = 1;
        if(x>0.50) led50 = 1;
        if(x>0.75) led75 = 1;

        //printf("%f\r\n", x);
        wait(0.50);
        
        com.printf("sattime:%f  V:%f  T:%f acc:%f,%f,%f lat:%f lon:%f\r\n",sattime.read(),bt,temp,ax,ay,az,latitude,longitude);
        wait(1);
        com.printf(" light sensor : %f\r\n",x);
        wait(0.50);

        float light_val=x;
        com.initialize();

        if (light_val>0.90) {
            break;
        }
        
    }
    

    float mx,my,mz;
    sensor.sen_mag(&mx,&my,&mz);

    while(1){
        com.xbee_receive(&rcmd,&cmdflag);
        char str[100];
        float mx1=mx;
        float my1=my;
        float mz1=mz;
        
        sensor.temp_sense(&temp);

        eps.vol(&bt);
        sensor.gps_setting();


        int quality=0,stnum=0,gps_check=0;
        char ns='A',ew='B',aunit='m';
        float time=0.0,latitude=0.0,longitude=0.0,hacu=0.0,altitude=0.0;
        int flag=0;
        if (flag == 0) {
            sensor.gga_sensing(&time, &latitude, &ns, &longitude, &ew, &quality, &stnum, &hacu, &altitude, &aunit, &gps_check);
            if((gps_check==0)|(gps_check==1)) {
                //pc.printf("latitude: %f  longitude: %f\r\n",latitude,longitude);
                flag=1;
            }
        }


        com.printf("sattime: %f  V: %f  T: %d  acc : %f,%f,%f",sattime.read(),bt,temp,ax,ay,az);
        wait(0.50);
        com.printf("lat: %f lon: %f",latitude, longitude);
        wait(0.5);
        com.printf("mag : %f,%f,%f\r\n",mx,my,mz);
        wait(0.5);
        fprintf(fp,"sattime=%f lat: %f lon: %f mag : %f,%f,%f\r\n",sattime.read(),latitude, longitude,mx,my,mz);
        wait(0.5);

        sensor.sen_mag(&mx,&my,&mz);
        
        if (1000<mx-mx1 or -1000>mx-mx1 ) {
            wait(10);
            com.printf("magnetic anomaly\r\n");
            wait(20);
            com.printf("p");
            wait(20);
            com.printf("\r\n");
        }
        

        com.xbee_receive(&rcmd,&cmdflag);
        if (cmdflag == 1) {
            if (rcmd == 'c') {
                fclose(fp);

                fp = fopen("/sd/mydir/test.txt","r");
                if(fp == NULL) {
                    error("Could not open file for write\r\n");
                    com.printf("not find sdcards/r/n");
                }
                for(int j = 0; j < 10; j++) {
                    fgets(str,100,fp);
                    com.puts(str);
                }
                fclose(fp);
                
            }
            com.initialize();
        }
    }
}
#include "mbed.h"
#include "Dht11.h"  
#include"Air_Quality.h"
#include "MPL3115A2.h"
#include"HEPTA_EPS.h"
#include"HEPTA_SENSOR.h"
#include"HEPTA_COM.h"
#include"HEPTA_CDH.h"

HEPTA_CDH cdh(p5, p6, p7, p8, "sd"); //SD
RawSerial pc(USBTX, USBRX); 
AnalogIn light(p18);              // light sensor
I2C i2c(p28, p27);                   // I2c(sda, scl)
HEPTA_EPS eps(p16,p26);              // eps
HEPTA_SENSOR sensor(p17,p28,p27,0x19,0x69,0x13,p13, p14,p25,p24);
HEPTA_COM com(p9,p10,9600);          
Timer sattime;

int main() {
    char c;
    float temp;
    int i=0,rcmd=0,cmdflag=0;
    float bt,ax,ay,az;
    int flag=0;
    sattime.start();

    //SD check
    FILE *fp = fopen("/sd/mydir/test.txt","w");
    if(fp == NULL) {
        error("Could not open file for write\r\n");
        com.printf("not find sdcards/r/n");
    }
   
    while(1){
        com.xbee_receive(&rcmd,&cmdflag);
        sensor.temp_sense(&temp);    //temprature
        sensor.sen_acc(&ax,&ay,&az); //acceleration
        eps.vol(&bt);                //battery voltage
        sensor.gps_setting();
        com.printf("1");
        int quality=0,stnum=0,gps_check=0;
        char ns='A',ew='B',aunit='m';
        float time=0.0,latitude=0.0,longitude=0.0,hacu=0.0,altitude=0.0;
        for(int i=1; i<100; i++) {
            com.printf("2");
            sensor.gga_sensing(&time, &latitude, &ns, &longitude, &ew, &quality, &stnum, &hacu, &altitude, &aunit, &gps_check);
            if((gps_check==0)|(gps_check==1)) {
                //com.printf("GPGGA,%f,%f,%c,%f,%c,%d,%d,%f,%f,%c\r\n",time,latitude,ns,longitude,ew,quality,stnum,hacu,altitude,aunit);
                break;
            }

        }
        
        com.printf("time:%f V:%f T:%f acc:%f,%f,%f lat:%f lon:%f\r\n",sattime.read(),bt,temp,ax,ay,az,latitude,longitude);

        if (rcmd == 'a') {
                com.printf("get a\r\n");
                for(int i=0;i<100;i++){
                    sensor.temp_sense(&temp);    //temprature
                    sensor.sen_acc(&ax,&ay,&az); //acceleration
                    eps.vol(&bt);                //battery voltage
                    sensor.gps_setting();
                    int quality=0,stnum=0,gps_check=0;
                    char ns='A',ew='B',aunit='m';
                    float time=0.0,latitude=0.0,longitude=0.0,hacu=0.0,altitude=0.0;
                    for(int i=1; i<100; i++) {
                        sensor.gga_sensing(&time, &latitude, &ns, &longitude, &ew, &quality, &stnum, &hacu, &altitude, &aunit, &gps_check);
                        if((gps_check==0)|(gps_check==1)) {
                            break;
                        }
                    }
                    float light_val=light;
                    com.printf("sattime:%f V:%f T:%f acc:%f,%f,%f lat:%f lon:%f light sensor:%f\r\n",sattime.read(),bt,temp,ax,ay,az,latitude,longitude,light_val);
                    if (light_val>0.5) {
                        flag=1;
                        break;
                    }
                }
            com.initialize();
        }

        //mission mode
        if (flag == 1) {
            for(int i;i<1000;i++){
                float mx,my,mz;
                char str[100];
                sensor.sen_mag(&mx,&my,&mz);
                sensor.temp_sense(&temp);
                eps.vol(&bt);
                sensor.gps_setting();
                int quality=0,stnum=0,gps_check=0;
                char ns='A',ew='B',aunit='m';
                float time=0.0,latitude=0.0,longitude=0.0,hacu=0.0,altitude=0.0;
                for(int i=1; i<100; i++) {
                    sensor.gga_sensing(&time, &latitude, &ns, &longitude, &ew, &quality, &stnum, &hacu, &altitude, &aunit, &gps_check);
                    if((gps_check==0)|(gps_check==1)) {
                        break;
                    }
                }
                
                com.printf("sattime:%f  V:%f  T:%d  acc:%f,%f,%f mag:%f,%f,%f\r\n",sattime.read(),bt,temp,ax,ay,az,mx,my,mz);
                fprintf(fp,"sattime=%f lat: %f lon: %f mag : %f,%f,%f\r\n",sattime.read(),latitude, longitude,mx,my,mz);
                float mx1=mx;
                float my1=my;
                float mz1=mz;

                sensor.sen_mag(&mx,&my,&mz);
                if (100<mx-mx1 or -100>mx-mx1 ) {
                    wait(1);
                    com.printf("magnetic anomaly\r\n");
                    wait(20);
                    com.printf("P");
                    wait(20);
                    com.printf("\r\n");
                }

                com.xbee_receive(&rcmd,&cmdflag);

                if (cmdflag == 1) {
                    if (rcmd == 'a') {
                        break;
                    }
                }

                if (cmdflag == 1) {
                    if (rcmd == 'b') {
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
            flag=0;
        }
    }
}
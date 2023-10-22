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
AnalogIn light(p15);              // light sensor
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
        int quality=0,stnum=0,gps_check=0;
        char ns='A',ew='B',aunit='m';
        float time=0.0,latitude=0.0,longitude=0.0,hacu=0.0,altitude=0.0;
        int start_time=0;
        /*
        for(int i=1; i<100; i++) {
            sensor.gga_sensing(&time, &latitude, &ns, &longitude, &ew, &quality, &stnum, &hacu, &altitude, &aunit, &gps_check);
            if((gps_check==0)|(gps_check==1)) {
                //com.printf("GPGGA,%f,%f,%c,%f,%c,%d,%d,%f,%f,%c\r\n",time,latitude,ns,longitude,ew,quality,stnum,hacu,altitude,aunit);
                break;
            }
        }
        */
        
        com.printf("time:%f V:%f T:%f acc:%f,%f,%f lat:%f lon:%f\r\n",sattime.read(),bt,temp,ax,ay,az,latitude,longitude);
        wait(0.2);

        if (rcmd == 'a') {
                com.printf("get a\r\nattitude mode\r\n");
                for(int i=0;i<100;i++){
                    sensor.temp_sense(&temp);    //temprature
                    sensor.sen_acc(&ax,&ay,&az); //acceleration
                    eps.vol(&bt);                //battery voltage
                    
                    sensor.gps_setting();
                    int quality=0,stnum=0,gps_check=0;
                    char ns='A',ew='B',aunit='m';
                    float time=0.0,latitude=0.0,longitude=0.0,hacu=0.0,altitude=0.0;
                    /*
                    start_time=sattime.read();
                    if(sattime.read()-start_time<10) {
                        sensor.gga_sensing(&time, &latitude, &ns, &longitude, &ew, &quality, &stnum, &hacu, &altitude, &aunit, &gps_check);
                        com.printf("1");
                    }
                    */
                    float light_val=light;
                    com.printf("time:%f V:%f T:%f acc:%f,%f,%f lat:%f lon:%f",sattime.read(),bt,temp,ax,ay,az,latitude,longitude);
                    com.printf(" light:%f\r\n",light_val);
                    wait(0.2);
                    if (light_val>0.5) {
                        com.printf("light value %f > 0.5\r\nattitude OK!\r\n",light_val);
                        flag=1;
                        break;
                    }
                }
            com.initialize();
        }
        
        //mission mode
        if (flag == 1) {
            com.printf("mission start\r\n");
            for(int i=0;i<1000;i++){
                float mx,my,mz;
                char str[100];
                sensor.temp_sense(&temp);
                sensor.sen_mag(&mx,&my,&mz);
                eps.vol(&bt);
                sensor.gps_setting();
                int quality=0,stnum=0,gps_check=0;
                char ns='A',ew='B',aunit='m';
                float time=0.0,latitude=0.0,longitude=0.0,hacu=0.0,altitude=0.0;
                /*
                for(int i=1; i<100; i++) {
                    sensor.gga_sensing(&time, &latitude, &ns, &longitude, &ew, &quality, &stnum, &hacu, &altitude, &aunit, &gps_check);
                    if((gps_check==0)|(gps_check==1)) {
                        break;
                    }
                }
                */
                com.printf("time:%f V:%f T:%f acc:%f,%f,%f",sattime.read(),bt,temp,ax,ay,az);
                com.printf(" mag:%f,%f,%f\r\n",mx,my,mz);
                fprintf(fp,"sattime=%f lat: %f lon: %f mag : %f,%f,%f\r\n",sattime.read(),latitude, longitude,mx,my,mz);
                wait(0.2);
                float mx1=mx;
                float my1=my;
                float mz1=mz;

                sensor.sen_mag(&mx,&my,&mz);
                if (100<mx-mx1 or -100>mx-mx1 ) {
                    com.printf("magnetic anomaly\r\n");
                    com.printf(" mag:%f,%f,%f ->  mag:%f,%f,%f\r\n",mx1,my1,mz1,mx,my,mz);
                    com.printf("Close TeraTerm & Run python Code\r\n");
                    wait(7);
                    com.printf("P");
                    wait(1);
                    com.printf("\r\nSent command P to GS\r\n");
                    wait(1);
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
                        com.printf("Past mission data in SDcard\r\n");
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
#include "mbed.h"
#include "math.h"

Serial pc(USBTX, USBRX); // tx, rx
DigitalOut myled1(LED1), myled2(LED2), myled3(LED3), myled4(LED4);
AnalogIn ain1(p15);
float Vin1;
Ticker flip_pin;
void pin_flip();
volatile float freq = 0.01;
const unsigned long MAXLEN = 100;
bool trigger = false;
uint16_t buffer[MAXLEN];
LocalFileSystem local("local");               // Create the local filesystem under the name "local"
 
void sample(){
    if(trigger == true){
        //buffer[0] = 0xA0A0;
        for(int i=0;i<MAXLEN;i++){
            buffer[i] = ain1.read_u16();
            wait(freq);
        }
        
        for(int i=0;i<MAXLEN;i++){
            uint8_t c;
            c = buffer[i] >> 8;
            pc.putc(c);
            c = buffer[i] >> 0;
            pc.putc(c);
        }
     trigger = false;
    }
    /*
    pc.printf("%lf\n",freq);
    wait(0.1);*/
}

int main() {
    unsigned char c = '\0';
    pc.baud(115200);
    //pc.attach(&pcInterruptHandler,RxIrq);
    while(1){
        sample();
        if (pc.readable()){    
            c = pc.getc();
            //pc.printf("%c",c);
            if(c=='f'){
                //pc.printf("Freq:");
                pc.scanf("%f",&freq);
                //pc.printf("freq=%f\n",freq);
            }
            else if(c=='t'){
                //pc.printf("*TRIG*\n");
                trigger = true;
            }
         }

    }
}



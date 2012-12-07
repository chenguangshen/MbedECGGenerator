#include "mbed.h"
#include "math.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);
AnalogOut output(p18);
Serial pc(USBTX, USBRX);
AnalogIn anain(p20);
LocalFileSystem local("local");
Ticker ticker;

float buffer[2000];
float pvc[2000];

bool pvcflag = false;

int buffer_length;
int pvc_length;
int count = 0;
int cur_length;
int g_bpm, g_amp, g_fs;
double pvc_bpm;
int gain;

// I am not going to save all the 0s,
// these indexes point to locations of non-zero values
int i_buf_1, i_buf_2, i_buf_3, i_buf_4;
int i_pvc_1, i_pvc_2, i_pvc_3, i_pvc_4;

double g_d, g_RR;
double earlyfactor = 0.25; 
double PVCwidth = 0.12;
double pvc_chance = 0.2;

int generate_pvc(double qrswidth, double bpm, int fs, double amp);

double gen_random (){
    double ain = anain.read();
    return ((int)(ain * 100000) % 100) / 100.0;
}

void output_signal() {
    if (!pvcflag) {
        if (count  >= i_buf_1 && count < i_buf_2) {
            output.write(buffer[count - i_buf_1]);
        }
        else if (count >= i_buf_3 && count < i_buf_4) {
            output.write(buffer[count - i_buf_3 + i_buf_2 - i_buf_1 + 1]);
        }
        else {
            output.write(0.1);
        }
    }
    else {
        if (count  >= i_pvc_1 && count < i_pvc_2) {
            output.write(pvc[count - i_pvc_1]);
        }
        else if (count >= i_pvc_3 && count < i_pvc_4) {
            output.write(pvc[count - i_pvc_3 + i_pvc_2 - i_pvc_1 + 1]);
        }
        else {
            output.write(0.1);
        }
    }
    count++;
    if (count == cur_length) {
        ticker.detach();
        count = 0;
        if ((gen_random() <= pvc_chance) && !pvcflag) {
            pvcflag = true;
            double ran_amp = g_amp + 0.4 * gen_random() * g_amp;
//            double new_bpm = (double)g_bpm / ((1 - earlyfactor) * g_RR - 0.4375 * PVCwidth);
//            if (new_bpm < 0) {
//                new_bpm = (double)g_bpm;
//            }
            pvc_length = generate_pvc(g_d, g_bpm, g_fs, ran_amp);
            cur_length = pvc_length;
            ticker.attach_us(&output_signal, 1000000.0 / (pvc_bpm / 60.0) / (double)pvc_length);
        }
        else {
            pvcflag = false;
            cur_length = buffer_length;
            ticker.attach_us(&output_signal, 1000000.0 / ((double)g_bpm / 60.0) / (double)buffer_length);
        }
    }
}

int generate_pvc(double qrswidth, double bpm, int fs, double amp) {
    pvc_bpm = bpm;
    //FILE *fp1 = fopen("/local/pvc.txt", "w");
    memset(pvc, 0, sizeof(pvc));
    
    double d = 0.1;
    int at = 0.5 * amp;
    double RR = 60.0 / bpm;    
    double d1 = 0.4375 * d;
    double d2 = 0.5 * d;
    double d3 = d - (d1 + d2);
    double dt = 0.180;
    double qt = 0.35;
    double deadspace = RR - qt;
    if (deadspace < 0) {
        return -1;
    }
    double t1;
    double t2;
    
    int pvc_count = 0;
    
    t1 = 0;
    t2 = t1 + d1;
    i_pvc_1 = 0;
    int i_t1 = ceil(fs * t1) + 1;
    int i_t2 = ceil(fs * t2) + 1;
    //fprintf(fp1, "t1: %lf t2: %lf i_t1: %d i_t2: %d\n", t1, t2, i_t1, i_t2); 
    double left = 0;
    double right = 0.875 * amp;
    double m1 = (right - left) / (t2 - t1);
    //fprintf(fp, "m1: %lf left: %lf right: %lf\n", m1, left, right);
    for (int i = i_t1 - 1; i < i_t2; i++) {
        pvc[pvc_count++] = i * m1 / fs - m1 * t1 + left;
        //fprintf(fp1, "m1: %lf i: %d i_t1: %d i_t2 %d res: %lf\n", m1, i, i_t1, i_t2, buffer[i]);
    }
    //fprintf(fp1, "1: %d %d\n", i_t1, i_t2);
        
    t1 = t2;
    t2 = t1 + d2;
    i_t1 = ceil(fs * t1) + 1;
    i_t2 = ceil(fs * t2) + 1;
    left = right;
    right = -0.125 * amp;
    double m2 = (right - left) / (t2 - t1);
    for (int i = i_t1 - 1; i < i_t2; i++) {
        pvc[pvc_count++] = i * m2 / fs - m2 * t1 + left; 
    }    
    //fprintf(fp1, "2: %d %d\n", i_t1, i_t2);
        
    t1 = t2;
    t2 = t1 + d3;
    i_t1 = ceil(fs * t1) + 1;
    i_t2 = ceil(fs * t2) + 1;
    //fprintf(fp1, "3: %d %d\n", i_t1, i_t2);
    left = right;
    right = 0;
    double m3 = (right - left) / (t2 - t1);
    if (i_t1 < i_t2) {
        for (int i = i_t1 - 1; i < i_t2; i++) {
            double cur = i * m3 / fs - m3 * t1 + left;
            //fprintf(fp1, "m3: %lf i: %d i_t1: %d i_t2 %d res: %lf\n", m3, i, i_t1, i_t2, cur);
            if (cur < 0) {
                pvc[pvc_count++] = cur;
                i_pvc_2 = i + 1;
            }
            else{ 
                i_pvc_2 = i;
                break;
            }
        }
    }
    else if (i_t1 == i_t2) {
        pvc[pvc_count++] = left; 
        i_pvc_2 = i_t1;
    }
        
    t1 = t2;
    t2 = t1 + qt  - (dt + t2);
    i_t1 = ceil(fs * t1) + 1;
    i_t2 = ceil(fs * t2) + 1;
//    for (int i = i_t1 - 1; i < i_t2; i++) {
//        pvc[i] = 0;
//    }
    //fprintf(fp1, "4: %d %d\n", i_t1, i_t2);
        
    t1 = t2;
    t2 = t1 + dt;
    i_t1 = ceil(fs * t1) + 1;
    i_t2 = ceil(fs * t2) + 1;
    i_pvc_3 = i_t1;
    i_pvc_4 = i_t2;
    for (int i = i_t1; i < i_t2; i++) {
        double temp = -1.0 + (i - i_t1 + 1) * 2.0 / (i_t2 - i_t1);
        pvc[pvc_count++] = at * sqrt(1.0 - temp * temp);
        //fprintf(fp1, "i: %d i_t1: %d i_t2 %d res: %lf\n", i, i_t1, i_t2, buffer[i]);
    }
    //fprintf(fp1, "5: %d %d\n", i_t1, i_t2);

    t1 = t2;
    t2 = t1 + deadspace;
    i_t1 = ceil(fs * t1) + 1;
    i_t2 = ceil(fs * t2) + 1;
//    for (int i = i_t1 - 1; i < i_t2; i++) {
//        pvc[i] = 0;
//    }
    //fprintf(fp1, "6: %d %d\n", i_t1, i_t2);    
    
    int len = ceil((60.0 / bpm) * fs) + 1;
//    fprintf(fp1, "print pvc:len=%d\n", len);
//    fclose(fp1);
    for (int i = 0; i < pvc_count; i++) {
        pvc[i] = (pvc[i] / (1400.0) / 1000.0 * gain) + 0.1;
    }
    return len;
}

int generate_ecg(int fs, int bpm, int amp) {
    g_bpm = bpm;
    g_fs = fs;
    g_amp = amp;
    FILE *fp = fopen("/local/result.txt", "w");
    memset(buffer, 0, sizeof(buffer));
    double duration = (60.0 / (double)bpm - 0.35) + 60.0 / (double)bpm + 1.0 / fs;
    double d = 0.1;
    g_d = d;
    int at = 0.5 * amp;
    int org_amp = amp;
    double RR = 60.0 / bpm;
    g_RR = RR;
    double d1 = 0.4375 * d;
    double d2 = 0.5 * d;
    double d3 = d - (d1 + d2);
    double dt = 0.180;
    double qt = 0.35;
    double deadspace = RR - qt;
    if (deadspace < 0) {
        return -1;
    }
    double t1 = deadspace;
    double t2;
    double qrs_start;
    int length = ceil((60.0 / bpm) * fs) + 1;
    buffer_length = length;
    cur_length = length;
    int buf_count = 0;
    
    while (t1 + 60.0 / bpm + 1.0 / fs <= duration) {
        amp = org_amp;
        qrs_start = t1;
        t2 = t1 + d1;
        int i_t1 = ceil(fs * t1) + 1;
        int i_t2 = ceil(fs * t2) + 1;
        //fprintf(fp, "t1: %lf t2: %lf i_t1: %d i_t2: %d\n", t1, t2, i_t1, i_t2); 
        double left = 0;
        double right = 0.875 * amp;
        double m1 = (right - left) / (t2 - t1);
        //fprintf(fp, "m1: %lf left: %lf right: %lf\n", m1, left, right);
        
        i_buf_1 = i_t1 - 1;
        
        for (int i = i_t1 - 1; i < i_t2; i++) {
            buffer[buf_count++] = i * m1 / fs - m1 * t1 + left;
            //fprintf(fp, "m1: %lf i: %d i_t1: %d i_t2 %d res: %lf\n", m1, i, i_t1, i_t2, buffer[i]);
        }
        fprintf(fp, "1: %d %d\n", i_t1, i_t2);
        
        t1 = t2;
        t2 = t1 + d2;
        i_t1 = ceil(fs * t1) + 1;
        i_t2 = ceil(fs * t2) + 1;
        left = right;
        right = -0.125 * amp;
        double m2 = (right - left) / (t2 - t1);
        for (int i = i_t1 - 1; i < i_t2; i++) {
            buffer[buf_count++] = i * m2 / fs - m2 * t1 + left; 
        }    
        fprintf(fp, "2: %d %d\n", i_t1, i_t2);
        
        t1 = t2;
        t2 = t1 + d3;
        i_t1 = ceil(fs * t1) + 1;
        i_t2 = ceil(fs * t2) + 1;
        fprintf(fp, "3: %d %d\n", i_t1, i_t2);
        left = right;
        right = 0;
        double m3 = (right - left) / (t2 - t1);
        if (i_t1 < i_t2) {
            for (int i = i_t1 - 1; i < i_t2; i++) {
                double cur = i * m3 / fs - m3 * t1 + left;
                //fprintf(fp, "m3: %lf i: %d i_t1: %d i_t2 %d res: %lf\n", m3, i, i_t1, i_t2, cur);
                if (cur < 0) {
                    buffer[buf_count++] = cur;
                    i_buf_2 = i + 1;
                }
                else{ 
                    i_buf_2 = i;
                    break;
                }
            }
            
        }
        else if (i_t1 == i_t2) {
            buffer[buf_count++] = left; 
            i_buf_2 = i_t1 + 1;
        }
        
        t1 = t2;
        t2 = t1 + qt + qrs_start - (dt + t2);
        i_t1 = ceil(fs * t1) + 1;
        i_t2 = ceil(fs * t2) + 1;
//        for (int i = i_t1 - 1; i < i_t2; i++) {
//            buffer[i] = 0;
//        }
        fprintf(fp, "4: %d %d\n", i_t1, i_t2);
        
        t1 = t2;
        t2 = t1 + dt;
        i_t1 = ceil(fs * t1) + 1;
        i_t2 = ceil(fs * t2) + 1;
        for (int i = i_t1; i < i_t2; i++) {
            double temp = -1.0 + (i - i_t1 + 1) * 2.0 / (i_t2 - i_t1);
            buffer[buf_count++] = at * sqrt(1.0 - temp * temp);
            //fprintf(fp, "i: %d i_t1: %d i_t2 %d res: %lf\n", i, i_t1, i_t2, buffer[i]);
        }
        i_buf_3 = i_t1;
        i_buf_4 = i_t2;
        fprintf(fp, "5: %d %d\n", i_t1, i_t2);
        
        t1 = t2;
        t2 = t1 + deadspace;
        i_t1 = ceil(fs * t1) + 1;
        i_t2 = ceil(fs * t2) + 1;
//        for (int i = i_t1 - 1; i < i_t2; i++) {
//            buffer[i] = 0;
//        }
        fprintf(fp, "6: %d %d\n", i_t1, i_t2);
        t1 = t2;
    }
    fprintf(fp, "%d %d %d %d:\n", i_buf_1, i_buf_2, i_buf_3, i_buf_4);
    fprintf(fp, "print buffer:\n");
    for (int i = 0; i < buf_count; i++) {
        buffer[i] = buffer[i] / (1400.0) / 1000.0 * gain + 0.1;
        fprintf(fp, "#%d : %f\n", i, buffer[i]);
    }
    fclose(fp);
    
    ticker.attach_us(&output_signal, 1000000.0 / ((double)bpm / 60.0) / (double)length);
    return 0;
}

void Rx_interrupt() {
    if (pc.readable()) {
        ticker.detach();
        int new_fs = g_fs, new_bpm = g_bpm, new_amp = g_amp;
        pc.scanf("%d %d %d %d", &new_fs, &new_bpm, &new_amp, &gain);
        pvcflag = false;
        count = 0;
        int res = generate_ecg(new_fs, new_bpm, new_amp);
        if (res == 0) {
            led1 = 1;
        }
        else {
            led2 = 1;
        }
    }
}

int main() {
    pc.attach(&Rx_interrupt, RxIrq);
    while (1) {
    
    }
}
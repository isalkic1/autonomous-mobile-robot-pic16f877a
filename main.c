// Algoritam upravljanja pic16f877a 
 
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator) 
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled) 
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled) 
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled) 
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming 
Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming) 
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM 
code protection off) 
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection 
off; all program memory may be written to by EECON control) 
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection 
off) 
 
#include <stdio.h> 
#include <xc.h> 
 
#define _XTAL_FREQ 8000000 
 
#define testbit(var,bit) ((var) & (1<<(bit))) // AND sa jedinicama 
#define setbit(var,bit) ((var) |= (1<<(bit))) // dodjela OR sa jedinicama 
#define clrbit(var,bit) ((var) &= ~(1<<(bit))) // dodjela AND sa nulama 
 
int stotinke=0, brojac=0; 
 
void init_PWM_lijevi(char D) 
{ 
    PR2=124; 
    T2CON=2; 
    CCPR2L=125*D/100; 
    CCP2CON=12; 
    TMR2=0; 
    T2CONbits.TMR2ON=1; 
} 
 
void init_PWM_desni(char D) 
{ 
    PR2=124; 
    T2CON=2; 
    CCPR1L=125*D/100; 
    CCP1CON=12; 
    TMR2=0; 
    T2CONbits.TMR2ON=1; 
} 
 
void init_TMR0() 
{ 
    T0CS=0; 
    PSA=0; 
    PS0=1; 
    PS1=1; 
    PS2=0; 
    TMR0IF=0; 
    TMR0IE=1; 
    GIE=0; 
} 
 
void init_analog() 
{ 
    // PORTA ce biti analogni 
    TRISA=0xFF; 
    PORTA=0x00; 
    // PORTE ce biti analogni (dodatni pinovi za A/D konverziju) 
    TRISE=0xFF; 
    PORTE=0x00; 
    // Lijevo poravnanje 
    ADCON1bits.ADFM=0; 
    // Interni RC oscilator za ADC 
    ADCON1bits.ADCS2=1; 
    ADCON0bits.ADCS1=1; 
    ADCON0bits.ADCS0=1; 
    // Vss za Vref-, Vdd za Vref+, svi <AN7:AN0> analogni ulazi 
    ADCON1bits.PCFG3=0; 
    ADCON1bits.PCFG2=0; 
    ADCON1bits.PCFG1=0; 
    ADCON1bits.PCFG0=0; 
    // Ukljucivanje ADC 
    ADCON0bits.ADON=1; 
    // Izbor AN0 na pocetku 
    ADCON0bits.CHS2=0; 
    ADCON0bits.CHS1=0; 
    ADCON0bits.CHS0=0; 
} 
 
char citajanalog(brporta) 
{ 
    // Izbor kanala za A/D konverziju 
    if(brporta==0) // AN0 
    { 
        CHS0=0; 
        CHS1=0; 
        CHS2=0; 
    } 
    else if(brporta==1) // AN1 
    { 
        CHS0=1; 
        CHS1=0; 
        CHS2=0; 
    } 
    else if(brporta==4) // AN4 
    { 
        CHS0=0; 
        CHS1=0; 
        CHS2=1; 
    } 
    GO=1; 
    while(GO); 
    return ADRESH; // rezultat konverzije se postavlja u ADRESH 
} 
 
char pretvori_udalj(char napon, const char *vrijednosti){ 
    char udaljenost, vrijednost; 
    int i, k=0; 
    vrijednost=vrijednosti[k]; 
    while(napon<vrijednost){ 
        k++; 
        vrijednost=vrijednosti[k]; 
    } 
    udaljenost=10; //gledamo od druge vrijednosti, udaljenost na 10 cm 
    for(i=0; i<k; i++) 
        udaljenost+=5; 
    return udaljenost; 
} 
 
void __interrupt() prekid(void) 
{ 
    if(TMR0IE && TMR0IF) 
    { 
        TMR0IF=0; 
        if(TMR0==0) 
            TMR0=129; 
        if(brojac<10) 
            brojac++; 
        else 
        { 
            brojac=0; 
            stotinke++; 
        } 
    } 
} 
 
void idi_naprijed() 
{ 
    char D=90; 
    init_PWM_lijevi(D); 
    init_PWM_desni(D); 
} 
 
void stani() 
{ 
    char D=0; 
    init_PWM_desni(D); 
    init_PWM_lijevi(D);    
} 
 
void rotacija_desno() 
{ 
    char D=90; 
    init_PWM_desni(0); 
    init_PWM_lijevi(D); 
    GIE=1; 
    while(stotinke<150); 
    GIE=0; 
    stotinke=0; 
    stani(); 
} 
 
void rotacija_lijevo() 
{ 
    char D=90; 
    init_PWM_desni(D); 
    init_PWM_lijevi(0); 
    GIE=1; 
    while(stotinke<160); 
    GIE=0; 
    stotinke=0; 
    stani(); 
} 
 
void main(void) 
{ 
    TRISCbits.TRISC1=0; // PWM desnog tocka 
    TRISCbits.TRISC2=0; // PWM lijevog tocka 
    TRISCbits.TRISC0=0; // C0 izlazni za smjer 
     
    PORTCbits.RC0=0; // smjer je uvijek 0 
    
    init_analog(); //inicijalizacija A/D konverzije 
    init_TMR0(); //inicijaliZacija TMR0 
    init_PWM_desni(0); // na pocetku nema kretanja 
    init_PWM_lijevi(0); // na pocetku nema kretanja 
     
    char dalj_naprijed,dalj_lijevo,dalj_desno,daljbcd; 
    
    //char[] vrijednosti=[2.28, 2.64, 1.93, 1.52, 1.24, 1.05, 0.92, 0.81, 0.73, 0.66, 0.61, 0.56, 0.52, 
0.49, 0.47, 0.45, 0.45, 0.43, 0.43, 0.42, 0.41, 0.41, 0.41, 0.40, 0.41]; 
    //char napon_senzor_naprijed[] = {1.68, 2.50, 1.80, 1.40, 1.16, 0.98, 0.88, 0.77, 0.71, 0.65, 
0.59, 0.56, 0.54, 0.49, 0.46, 0.44, 0.43, 0.42, 0.39, 0.40}; 
    //char napon_senzor_lijevo[] = {1.56, 2.49, 1.76, 1.36, 1.09, 0.92, 0.81, 0.71, 0.63, 0.59, 0.53, 
0.48, 0.46, 0.43, 0.39, 0.38, 0.36, 0.34, 0.33, 0.31}; 
    //char napon_senzor_desno[] = {1.55, 2.43, 1.70, 1.32, 1.07, 0.91, 0.84, 0.78, 0.76, 0.74, 
0.76, 0.68, 0.68, 0.72, 0.78, 0.82, 0.74, 0.78, 0.83, 0.84}; 
    
    char vrijednosti_senzor_naprijed[] = 
{127,91,71,59,49,44,39,36,33,30,28,27,24,23,22,21,21,19,20}; // odsjecena prva vrijednost 
    char vrijednosti_senzor_lijevo[] = 
{126,89,69,55,46,41,36,32,30,27,24,23,21,19,19,18,17,16,15}; // odsjecena prva vrijednost 
    char vrijednosti_senzor_desno[] = 
{123,86,67,54,46,42,39,38,37,38,34,34,36,39,41,37,39,42,42}; // odsjecena prva vrijednost 
    
    char zadana=20; // testna udaljenost 
    char prioritet=0; // prioritet desno -> 0, prioritet lijevo -> 1 
    
    while(1) 
    { 
        dalj_naprijed=citajanalog(0); 
        dalj_naprijed=pretvori_udalj(dalj_naprijed,vrijednosti_senzor_naprijed); // udaljenost sa 
senzora 
        if(dalj_naprijed>zadana) // treba ici naprijed 
        { 
            idi_naprijed(); 
            while(dalj_naprijed>zadana) 
            { 
                //idi naprijed i kontinuirana provjera udaljenosti ispred robota 
                dalj_naprijed=citajanalog(0); 
                dalj_naprijed=pretvori_udalj(dalj_naprijed,vrijednosti_senzor_naprijed); // udaljenost 
sa senzora 
            } 
            stani(); 
        } 
        else // ne treba ici naprijed - detektovana prepreka 
        {    
            // provjera prioriteta 
            if(prioritet==0) // desno 
            { 
                prioritet=1; // promjena prioriteta 
                dalj_desno=citajanalog(1); // provjera desnog senzora 
                dalj_desno=pretvori_udalj(dalj_desno,vrijednosti_senzor_desno); // udaljenost sa 
senzora 
                if(dalj_desno>zadana) // nema prepreka 
                    rotacija_desno(); 
            } 
            else if(prioritet==1) // lijevo 
            { 
                prioritet=0; // promjena prioriteta 
                dalj_lijevo=citajanalog(4); // provjera lijevog senzora 
                dalj_lijevo=pretvori_udalj(dalj_lijevo,vrijednosti_senzor_lijevo); // udaljenost sa 
senzora 
                if(dalj_lijevo>zadana) // nema prepreka 
                    rotacija_lijevo(); 
            } 
        } 
    } 
}
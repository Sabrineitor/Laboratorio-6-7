#include "io430.h"
#include "intrinsics.h"
#include "bsp.c"
#include "mmc.c"

unsigned char status = 1; 
unsigned int timeout = 0;

int buffer_in = 0; 
int buffer_out = 0; 

#define buffer_size 175 

struct audioStr {
  volatile unsigned char msb[buffer_size]; 
  volatile unsigned char lsb[buffer_size]; 
  unsigned char Rec; 
  unsigned char bufferBytes;  
  unsigned char Button;
} audio;

struct flashstr {
  unsigned int currentByte; 
  unsigned long currentSector; 
  unsigned long sectorInicial; 
} flashDisk;

unsigned char sdWrite(unsigned char data);


unsigned char sdWrite(unsigned char data)
{
  unsigned char status;

    mmcWriteByte(data);            

    flashDisk.currentByte++;

    if(flashDisk.currentByte==512+1) //Cambio de sector, LED y chispero
    {                              
      mmcWriteUnmount(flashDisk.currentSector);  

      flashDisk.currentByte=1;      
          
      flashDisk.currentSector++;    

      status=mmcWriteMount(flashDisk.currentSector);
    
      if(flashDisk.currentSector % 800 == 0) { //LED (on)                      
      P1OUT|=BIT2;
      }

      if((flashDisk.currentSector - 20) % 800 == 0) { //LED (off)                        
      P1OUT&=~BIT2;
      }
     
            if(flashDisk.currentSector % 5000 == 0) { //Chispero (on)                     
      P2OUT|=BIT0;
      }

      if((flashDisk.currentSector - 20) % 5000 == 0) { //Chispero (off)                        
      P2OUT&=~BIT0;
      }
      
    }   

    
    return status;          
   }

void Button(){

  P1DIR &= ~BIT3; //poner P1.3 como entrada
  P1REN |= BIT3; // habilito pull up/down p1.3
  P1OUT |= BIT3; // lo pongo como pull up
  //P1SEL &= 0x00; //
  P1IE |= BIT3; //Habilito la interrupción
  P1IES = BIT3; // Actva por frente de caída
  P1IFG &= ~BIT3; // Por si acaso se limpia la bandera
  //P1IES = BIT3; // Actva por frente de caída
  //__low_power_mode_3(); 
  //__enable_interrupt_3(); //probar
}

void ADC10(){
  ADC10CTL0 &= ~ENC; //(apagar antes de configurar)
  ADC10CTL0 = SREF_1 + REF2_5V + REFON + ADC10SHT_3 + ADC10ON + ADC10IE ;
  //ADC10CTL0 = SREF_2 + REFOUT + ADC10SHT_3 + ADC10ON + ADC10IE + REF2_5V + REFON;
  ADC10AE0 |=  BIT1;
}

void Timer(){
	CCTL0 = CCIE; 
  	TA0CTL = TASSEL_2 + ID_0 + MC_1; 
  	TACCR0 = 84; // TACCR0 + 1 = 62us -> 16kHz
  	TACCR1 = 29 ;
}

void Rec(){

    mmcReadUnmount(flashDisk.currentSector); 
    audio.bufferBytes=0;     
    audio.Rec=0;                
    flashDisk.currentByte=1;    
    status=mmcWriteMount(flashDisk.currentSector); 
}

int main( void )
{

  WDTCTL = WDTPW + WDTHOLD; 
  MainClockInit();
  SMCLKInit();
  
  P2DIR_bit.P0 = 1;
  __delay_cycles(1000000);
  P2OUT_bit.P0 ^= 1;
  P1DIR_bit.P2 = 1;
  __delay_cycles(1000000);
  P1OUT_bit.P2 ^= 1;
  __delay_cycles(1000000);
  P1OUT_bit.P2 ^= 1;
  __delay_cycles(1500000);

  while (status != 0){ //Inicialización de la SD 
    status = disk_initialize(); 
    timeout++;
    P1OUT_bit.P2 ^= 1;
    __delay_cycles(500000);

    if (timeout == 150){
      P1OUT_bit.P2 ^= 1;
      __delay_cycles(250000);
      P1OUT_bit.P2 ^= 1;
      __delay_cycles(250000);
      P1OUT_bit.P2 ^= 1;
      __delay_cycles(250000);
      P1OUT_bit.P2 ^= 1;
    }
  }
  P1OUT_bit.P2 ^= 1; 

  
  ADC10();// Seteo del ADC

  Rec(); // Seteo del Sector
   
  flashDisk.currentSector=0;
  
  Timer();// Seteo del Timer

  Button(); // Seteo del Botón
  
  __enable_interrupt(); // Habilitación de las interrupciones 

  while(1){ 
        if(audio.Rec){ 
          if(audio.bufferBytes>0){   
        	buffer_out = buffer_in-audio.bufferBytes+1;
          	if(buffer_out < 0)buffer_out += buffer_size;
        		status=sdWrite(audio.msb[buffer_out]);
        		status=sdWrite(audio.lsb[buffer_out]);
        		audio.bufferBytes--;    
      		}
      	}
  }
}

#pragma vector = PORT1_VECTOR // Interrupción del Botón (Cuando se toca el botón)
__interrupt void PORT1(void)
{
  if( (P1IFG & BIT3) != 0) // chequea la interrupción
  {
    if (audio.Rec == 0){
    	audio.Rec = 1;
    }
    P1IFG &= ~BIT3; // clears P1.3 interrupt flag
  }

} 

#pragma vector=TIMER0_A0_VECTOR // Interrupción activación de conversión del ADC10 (Cuando el timer llega a TACCR0)
__interrupt void interrupt_timer(void)
{     
    ADC10CTL0 &= ~ENC;
    ADC10CTL1 &= ~0xFFFF;
    ADC10CTL1= CONSEQ_0 + INCH_1 ;
    ADC10CTL0 |= ENC + ADC10SC; 
}

#pragma vector=ADC10_VECTOR // Interrupción de rutina de servicio ADC10 (Cuando el ADC10 completa una medición)
__interrupt void ADC10_ISR(void)
{
  audio.bufferBytes++;         
  buffer_in = buffer_in + 1;
  if(buffer_in == buffer_size) buffer_in =  0;
  while (ADC10CTL1 & BUSY);
  audio.msb[buffer_in]=(ADC10MEM >>2);       
  audio.lsb[buffer_in]=ADC10MEM & 0x03;  
}
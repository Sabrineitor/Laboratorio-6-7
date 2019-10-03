// Funciona a ambas frec del reloj interno. BOTON + LED por sector

#include "io430.h"
#include "intrinsics.h"
#include "bsp.c"
#include "mmc.c"

unsigned char status = 1; 
unsigned int timeout = 0;

int numero = 1;
int aux;

// Buffer
int buffer_in = 0; //Index entrada
int buffer_out = 0; //Index salida

#define buffer_size 175 // Tamaño buffer
#define MedirSectores 400 // Cantidad de sectores por medicion
int inicio;

struct audioStr {
  volatile unsigned char msb[buffer_size]; //stores one 8 bit audio output sample
  volatile unsigned char lsb[buffer_size]; //guarda los 2 bits perdidos (10bit ADC -> 8bit audio)
  unsigned char Rec; //audio record or play?
  unsigned char bufferBytes;  //number of audio bytes available
  unsigned char Button;
} audio;

struct flashstr {
  unsigned int currentByte; //byte counter for current 512byte sector
  unsigned long currentSector; //current sector
  unsigned long sectorInicial; //Sector inicial de la medicion
} flashDisk;

unsigned char sdWrite(unsigned char data);


unsigned char sdWrite(unsigned char data)
{
  unsigned char status;

    mmcWriteByte(data);             //(Función de la libreria) Escribe audio en el sector

    flashDisk.currentByte++;        //+1 en el contador de byte

    if(flashDisk.currentByte==512+1) //Llegamos al final de sector
    {                              
      mmcWriteUnmount(flashDisk.currentSector);  //(Función de la libreria) Cierra el sector

      flashDisk.currentByte=1;      //Resetea el contador de byte a 1
          
      flashDisk.currentSector++;    //Va al siguiente sector (+1 al contador de sector)

      status=mmcWriteMount(flashDisk.currentSector);//Se monta por sector! (BYTE O SECTOR?)
    
      if(flashDisk.currentSector % 60 == 0) { //LED (on)                         
      P1OUT|=BIT3;
      }

      if((flashDisk.currentSector - 20) % 60 == 0) { //LED (off)                        
      P1OUT&=~BIT3;
      }
    
    
    }   

    
    return status;           

}

void Button(){

  P1DIR &= ~BIT2; //poner P1.3 como entrada
  P1REN |= BIT2; // habilito pull up/down p1.3
  P1OUT |= BIT2; // lo pongo como pull up
  P1SEL &= 0x00; //
  P1IE |= BIT2; //Habilito la interrupción
  P1IFG &= ~BIT2; // Por si acaso se limpia la bandera
  P1IES = BIT2; // Actva por frente de caída
  //__low_power_mode_2();
  //__enable_interrupt_2();
  
    
}


void Rec(){

    mmcReadUnmount(flashDisk.currentSector);  //Cierra el sector (Buscar bien por que adentro pones el byte)
    
    //------write audio to SD card....
    //setup the write
    audio.bufferBytes=0;        //Elimina valores viejos en el buffer
    audio.Rec=0;                //enable recording code in WDT routine.....
    flashDisk.currentByte=1;    //Byte actual = 1
     //we start with sector 1 so we can store meta data in sector 0....
                               // Modificar para escribir en sector posterior al ultimo de la medicion anterior
    
    status=mmcWriteMount(flashDisk.currentSector); 
}

int main( void )
{
  //audio.Rec = 1;
  
  WDTCTL = WDTPW + WDTHOLD; //(Tiene que estar siempre -\(^.^)/- )
 
  // Arrancan los relojes, ver bsp.c
  MainClockInit();
  SMCLKInit();
   
  // Seteo el puerto 1.4 como salida logica para controlar un led
  P1DIR_bit.P3 = 1;//LED en 1.4 para ver cuando deja de andar
  __delay_cycles(20000);
  P1OUT_bit.P3 ^= 1;// Apaga
  __delay_cycles(20000);
  P1OUT_bit.P3 ^= 1;// Prende
  __delay_cycles(25000);
 
  // Initialization of the MMC/SD-card
  // creemos que esto se ejecuta mientras los errores en sdWrite sean distintos de cero
  while (status != 0){
    status = disk_initialize(); // Inicializa el disco
    timeout++;
    P1OUT_bit.P3 ^= 1;//Titila
    __delay_cycles(5000);
    // Try 150 times till error

    //porque 150?
    if (timeout == 150){
      P1OUT_bit.P3 ^= 1;
      __delay_cycles(5000);
      P1OUT_bit.P3 ^= 1;
      __delay_cycles(2500);
      P1OUT_bit.P3 ^= 1;
      __delay_cycles(2500);
      P1OUT_bit.P3 ^= 1;
      __low_power_mode_3();
    }
  }
  P1OUT_bit.P3 ^= 1; //Prende el led
 
  /*
  //PONEMOS UN BOTON!!!!!
  P1OUT=BIT2;
  P1IES=0;
  P1IFG=0;
  P1IE=BIT2;
  */
  //setup the ADC10
  ADC10CTL0 &= ~ENC; // Disable Conversion (apagar antes de configurar)
  ADC10CTL0 = SREF_1 + REF2_5V + REFON + ADC10SHT_0 + ADC10ON + ADC10IE + ADC10SR; // Rango, sample and hold time, ADC10ON, interrupt enabled saque refout
  __delay_cycles(3000);
  ADC10AE0 |=  BIT1;     //  El canal donde grabamos por lo que vimos es la patita 4 (bit2)
  
 
  //ADC10CTL0 |= ENC;
  Rec(); //Prepara la tarjeta sd (monta el primer sector).
  
  
  flashDisk.currentSector=0;
  
  
  // Seteo el timer:
  CCTL0 = CCIE; //Habilita las interrupciones del timer
  TA0CTL = TASSEL_2 + ID_0 + MC_1; //Configura la fuente  SMCLK (Con igual frecuecia que el reloj y modo UP) (Clear + Source + Divisor + Modo + Interrupt)
  TACCR0 = 61; // Le dice hasta cuando cuenta (Cuenta hasta TACCR0 + 1 = 62us -> 16kHz)
  TACCR1 = 29 ;
  
  Button(); //preparo el boton
  
  __enable_interrupt(); //Habilita las interrupciones (sacamos la interupcion del trigger)
                        // la otra interrupcion es para cuando se llena el buffer (creemos)
  //audio.Rec = 1;
  
  while(1){ //(Eso significa que siempre se hace?)
        if(audio.Rec){ // Si estamos en modo grabar
        if(audio.bufferBytes>0){   //There is a byte to be written in the record buffer
        buffer_out = buffer_in-audio.bufferBytes+1;
          if(buffer_out < 0)buffer_out += buffer_size;
        status=sdWrite(audio.msb[buffer_out]);//write the byte, get the result code in status
        status=sdWrite(audio.lsb[buffer_out]);//escribo el segundo byte, con los digitos menos significativos
                               //status=99 when the card is full (or reaches a predetermined limit)
        audio.bufferBytes--;    //subtract one from the buffer counter
      }
      }
         }
}


// empiezan las interupciones :)
//BOTON INTERRUPCION
#pragma vector = PORT1_VECTOR
__interrupt void PORT1(void)
{
  if( (P1IFG & BIT2) != 0) // chequea la interrupción
  {
  //audio.Button=1; //set the button value to true
  audio.Rec = 1;
  
  P1IFG &= ~BIT2; // clears P1.3 interrupt flag
  }

} 

// Cada vez que el timer llega a TACCR0 corre el siguiente codigo
// Lo que hace es activar la conversion del ADC10
#pragma vector=TIMER0_A0_VECTOR
__interrupt void interrupt_timer(void)
{
  if(audio.Rec){                 //if in record mode...trigger ADC to get audio sample for recording
    //if(audio.canal){            //Selecciono 1 de los dos canales esto se va porque tenemos un solo canal 
    ADC10CTL0 &= ~ENC;
    ADC10CTL1 &= ~0xFFFF;
    ADC10CTL1= CONSEQ_0 + INCH_1 + ADC10SSEL_0 + ADC10DIV_0 + SHS_0;;
    //audio.canal = 0;          // Cambio de canal para la proxima
    ADC10CTL0 |= ENC + ADC10SC; //Set bit to start conversion
                                //Aca interrumpe el ADC10 y va al otro interrupt (abajo)
  }
}

//ADC10 interrupt service routine
//when the ADC10 completes an audio sample measurement, this code runs
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  audio.bufferBytes++;          //new byte in the buffer (hay un byte mas para transmitir)
  buffer_in = buffer_in + 1;
  if(buffer_in == buffer_size) buffer_in =  0;
  while (ADC10CTL1 & BUSY);// espero a que se desocupe
  // El ADC10 guarda la medicion de 10 bits en ADC10MEM
  audio.msb[buffer_in]=(ADC10MEM>>2);       //convert audio to 8 bit sample....  >> binary shift -> mueve los bits 2 lugares
  audio.lsb[buffer_in]=ADC10MEM & 0x03;
}

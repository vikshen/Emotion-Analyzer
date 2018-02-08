// Target: C8051F02x
// Tool chain: Keil C51 7.50 / Keil EVAL C51
// Command Line: None
//------------------------------------------------------------
// Includes
//------------------------------------------------------------
#include <c8051f020.h> // SFR declarations
#include <stdio.h>
//------------------------------------------------------------
// 16-bit SFR Definitions for 'F02x
//------------------------------------------------------------
sfr16 ADC0 = 0xbe; // ADC0 data
sfr16 RCAP2 = 0xca; // Timer2 capture/reload
sfr16 RCAP3 = 0x92; // Timer3 capture/reload
sfr16 TMR2 = 0xcc; // Timer2
sfr16 TMR3 = 0x94; // Timer3
//------------------------------------------------------------
// Global Constants
//------------------------------------------------------------
#define BAUDRATE 9600 // Baud rate of UART in bps
#define SYSCLK 22118400 // External Crystal Osc clock
#define INT_DEC 256 // Integrate and decimate ratio
#define SAR_CLK 2500000 // Desired SAR clock speed
#define SAMPLE_RATE 50000 // Sample frequency in Hz
#define k 6000000 // 3000000
#define SAMPLE_DELAY 1200
#define ANALOG_INPUTS 8 // Number of AIN pins to
//measure (min=1, max=8)
//------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------
void OSCILLATOR_Init (void);
void PORT_Init (void);
void UART0_Init (void);
void ADC0_Init (void);
void TIMER3_Init (int counts);
void TIMER0_Init (void);
void ADC0_ISR (void);
void TIMER3_ISR (void);
void INT0_ISR(void);
void UART0_ISR(void);
void TIMER0_ISR (void);
void small_delay (char d); // 8 bit,about 0.34us per count

// @22.1MHz

void large_delay (char d); // 16 bit, about 82us per count

// @22.1MHz

void huge_delay (char d); // 24 bit, about 22ms per count



91

// @22.1MHz

//------------------------------------------------------------
// Global Variables
//------------------------------------------------------------
long Result[ANALOG_INPUTS]; // ADC0 decimated value,
// one for each analog
// input

unsigned char amux_input=0; // index of analog MUX

// inputs

unsigned char amux_convert=0;
unsigned long BPM_T_count_value = 0;
unsigned long BPM_Tick_Counter = 0;
unsigned int BPM = 0;
//------------------------------------------------------------
// main() Routine
//------------------------------------------------------------
void main (void)
{
unsigned char i;
EA = 0;
WDTCN = 0xde; // Disable watchdog timer
WDTCN = 0xad;
OSCILLATOR_Init ();
PORT_Init ();
UART0_Init ();
ADC0_Init ();
TIMER3_Init (SYSCLK/SAMPLE_RATE);
TIMER0_Init ();
AD0EN = 1; // Enable ADC
EA = 1; // Enable global
interrupts
while (1)
{
if (BPM_T_count_value !=0)
{
BPM = k/BPM_T_count_value;
}
else
BPM = 0;
for(i=0; i<ANALOG_INPUTS; i++)
{
}
printf("%ld#%4d#%ld#%ld#\n", Result[1]*2430/4095, BPM,
Result[7]*2430/4095,Result[6]*2430/4095);
huge_delay(12);// Settling time for the sensors
}
}
//------------------------------------------------------------



92

void OSCILLATOR_Init (void)
{
int i; // delay counter
OSCXCN = 0x67; // start external oscillator
// with 22.1184MHz crystal
for (i=0; i < 256; i++); // wait for oscillator to start
while (!(OSCXCN & 0x80)); // Wait for crystal osc. to

// settle

OSCICN = 0x88; // select external oscillator

// as SYSCLK
// source and enable missing
// clock
// detector

}
//-----------------------------------------------------------
void PORT_Init (void)
{
XBR0 = 0x04; // Route UART0 to crossbar
XBR1 = 0x14; // XBAR1: Initial Reset Value
XBR2 = 0x40; // Enable crossbar,weak pull-ups
P0MDOUT |= 0x01; // enable TX0 as a push-pull

// output

P1MDOUT |= 0x40; // enable LED as push-pull output
IT0 = 1; // INT0 edge triggered
EX0 = 1; // enabled external interrupt

// INT0

}
//------------------------------------------------------------
void UART0_Init (void)
{
SCON0 = 0x50; // SCON0: mode 1, 8-bit UART,

// enable RX

TMOD |= 0x20; // TMOD: timer 1, mode 2, 8-bit

// reload
TH1 = 0x70; // 9600
TR1 = 1; // start Timer1
CKCON |= 0x10; // Timer1 uses SYSCLK as time

// base.T1M=1
PCON |= 0x80; // SMOD00 = 1
TI0 = 1; // Indicate TX0 ready
RI0 = 0; // Clear received interrupt flag
}
//------------------------------------------------------------
Void ADC0_Init (void)
{
ADC0CN = 0x04; // ADC0 disabled; normal
// tracking mode; ADC0



93
// conversions are initiated
// on overflow of Timer3; ADC0
// data is right-justified
REF0CN = 0x07; // Enable temp sensor, on-chip
// VREF,and VREF output buffer
AMX0CF = 0x00; // AIN inputs are single-ended

// (default)

AMX0SL = 0x00; // Select AIN0.0 pin as ADC
// mux input ISR will change
// this to step through inputs
ADC0CF = 0x38; // ADC conversion clock =

// 2.5MHz

ADC0CF|= 0x00; // PGA gain = 1 (default)
EIE2 |= 0x02; // enable ADC interrupts
}
//------------------------------------------------------------
Void TIMER0_Init (void)
{
CKCON |= 0x08; // Timer 0 uses system clock

// T0M = 1

TMOD |= 0x02; // Timer 0 in Mode 2 auto

// reload

TL0 = 0xFF;
TH0 = 0x22; // Timer 0 reload (34)
TR0 = 1; // Start Timer 0
ET0 = 1; // Enable interrupt request

// for timer 0

}
//------------------------------------------------------------
Void TIMER3_Init (int counts)
{
TMR3CN = 0x02; // Stop Timer3; Clear TF3; set

// SysClk as time base
RCAP3 = -counts; // Init reload values
TMR3 = 0xffff; // Set to reload immediately
EIE2 |= 0x01; // Enable Timer3 interrupts
TMR3CN |= 0x04; // start Timer3
}
//------------------------------------------------------------

Void ADC0_ISR (void) interrupt 15
{
static unsigned int_dec=INT_DEC; // Integrate/decimate
// counter we post a new
// result when int_dec=0



94

static long accumulator[ANALOG_INPUTS] ={0L};

// Here's where we
// integrate the ADC
// samples from input
// AIN0.0

unsigned char i;
AD0INT = 0; // clear ADC conversion
// complete overflow
accumulator[amux_convert] += ADC0; // Read ADC value and
// add to running total
if(amux_convert == (ANALOG_INPUTS-1))// reset input index if

// the last input
//was just read

{
int_dec--; // Update decimation
// counter when last of
// the analog inputs
// sampled

}
if (int_dec == 0) // If zero, then post

// result

{
int_dec = INT_DEC; // Reset counter
for(i=0; i<ANALOG_INPUTS; i++)
{
Result[i] = accumulator[i] >> 8; //Copy decimated values

// into Result
accumulator[i] = 0L; // Reset accumulators
}
}
amux_convert = amux_input; // now that conversion
// results are stored,
// advance index to the
// analog input
// currently selected on
// the mux

}
//------------------------------------------------------------
void TIMER3_ISR(void) interrupt 14
{
TMR3CN &= ~0x80; // acknowledge interrupt
amux_input ++; // step to the next
// analog mux input
if(amux_input == ANALOG_INPUTS) // reset input index if
// the last input
{ // was just read
amux_input=0; // reset input index
// back to AIN0.0

}
AMX0SL = amux_input; // select the next input



95

// on the analog
// multiplexer

}
//------------------------------------------------------------
void TIMER0_ISR (void) interrupt 1
{
TF0 = 0; // reset timer overflow flag
BPM_Tick_Counter++; // tick count for BPM
}
//------------------------------------------------------------
void INT0_ISR(void) interrupt 0 // for BPM Calculation
{
if (35000 <= BPM_Tick_Counter && BPM_Tick_Counter <=150000)
//40-200 BPM range

{
BPM_T_count_value = BPM_Tick_Counter;
}
BPM_Tick_Counter = 0;
}
//------------------------------------------------------------
void small_delay(char d)
{
while (d--);
}
void large_delay(char d)
{
while (d--)
small_delay(255);
}
void huge_delay(char d)
{
while (d--)
large_delay(255);
}
//------------------------------------------------------------
// End Of File
//------------------------------------------------------------

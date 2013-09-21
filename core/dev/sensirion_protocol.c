/*********************************************
 * vim:sw=8:ts=8:si:et
 * This is the driver code for the sensirion temperature and
 * humidity sensor. 
 *
 * Based on ideas from the sensirion application note and modified
 * for atmega88/168. 
 * A major part of the code was optimized and as a result the compiled 
 * code size was reduced to 50%.
 *
 * Modifications by: Guido Socher 
 *
 * Note: the sensirion SHTxx sensor does _not_ use the industry standard
 * I2C. The sensirion protocol looks similar but the start/stop
 * sequence is different. You can not use the avr TWI module.
 *
 *********************************************/
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
                          //adr command r/w
#define STATUS_REG_W 0x06 //000 0011 0
#define STATUS_REG_R 0x07 //000 0011 1
#define MEASURE_TEMP 0x03 //000 0001 1
#define MEASURE_HUMI 0x05 //000 0010 1
#define RESET 0x1e        //000 1111 0

// physical connection:
// All sensors use the same SCK pin.
#define SETSCK1 PORTD|=(1<<PORTD0)
#define SETSCK0 PORTD&=~(1<<PORTD0)
#define SCKOUTP DDRD|=(1<<DDD0)
//
// Define where the DATA pin of each sensor is connected.
// Everything must be connected to port D
static uint8_t gSensor2dataDDR = DDD1;
static uint8_t gSensor2dataPORT = PORTD1;
static uint8_t gSensor2dataPIN = PIND1;
// the current measurement mode, storage area for each sensor is needed here:
static uint8_t gSensor2mode;
//static uint8_t overtime=0;
//static uint8_t cnt2step;
//
#define SETDAT1() PORTD|=(1<<gSensor2dataPORT)
#define SETDAT0() PORTD&=~(1<<gSensor2dataPORT)
#define GETDATA() (PIND&(1<<gSensor2dataPIN))
//
#define DMODEIN() DDRD&=~(1<<gSensor2dataDDR)
#define PULLUP1() PORTD|=(1<<gSensor2dataPIN)
#define DMODEOU() DDRD|=(1<<gSensor2dataDDR)

//pulswith long 3us
#define S_PULSLONG _delay_loop_1(16)
//pulswith short 1us
#define S_PULSSHORT _delay_loop_1(5)

//#ifdef ISR(TIMER2_COMPA_vect)
//#undef ISR(TIMER2_COMPA_vect)
//#endif

//ISR(TIMER2_COMPA_vect){
//cnt2step++;
//if(cnt2step>30)
//{
//overtime=1;
//}
//}


//void init_cnt2(void)
//{
//cnt2step=0;
//overtime=0;
//PRR0&=~(1<<PRTIM2);
//TIMSK2=(1<<OCIE2A);
//TCNT2=0;
//OCR2A=125;
//TCCR2A=(1<<WGM21);
//TCCR2B=(1<<CS22)|(1<<CS21)|(1<<CS20);
//}

// Compute the CRC8 value of a data set.
//
//  This function will compute the CRC8 of inData using seed
//  as inital value for the CRC.
//
//  This function was copied from Atmel avr318 example files.
//  It is more suitable for microcontroller than the code example
//  in the sensirion CRC application note.
//
//  inData  One byte of data to compute CRC from.
//
//  seed    The starting value of the CRC.
//
//  return The CRC8 of inData with seed as initial value.
//
//  note   Setting seed to 0 computes the crc8 of the inData.
//
//  note   Constantly passing the return value of this function 
//         As the seed argument computes the CRC8 value of a
//         longer string of data.
//
uint8_t computeCRC8(uint8_t inData, uint8_t seed)
{
    uint8_t bitsLeft;
    uint8_t tmp;

    for (bitsLeft = 8; bitsLeft > 0; bitsLeft--)
    {
        tmp = ((seed ^ inData) & 0x01);
        if (tmp == 0)
        {
            seed >>= 1;
        }
        else
        {
            seed ^= 0x18;
            seed >>= 1;
            seed |= 0x80;
        }
        inData >>= 1;
    }
    return seed;    
}

// sensirion has implemented the CRC the wrong way round. We
// need to swap everything.
// bit-swap a byte (bit7->bit0, bit6->bit1 ...)
uint8_t bitswapbyte(uint8_t byte)
{
        uint8_t i=8;
        uint8_t result=0;
        while(i){ 
		result=(result<<1);
                if (1 & byte) {
			result=result | 1;
                }
		i--;
		byte=(byte>>1);
        }
	return(result);
}

// writes a byte on the Sensibus and checks the acknowledge
char s_write_byte(uint8_t value)
{
        uint8_t i=0x80;
        uint8_t error=0;
        DMODEOU(); 
        while(i){ //shift bit for masking
                if (i & value) {
                        SETDAT1(); //masking value with i , write to SENSI-BUS
                }else{ 
                        SETDAT0();
                }
                SETSCK1; //clk for SENSI-BUS
                S_PULSLONG;
                SETSCK0;
                S_PULSSHORT;
                i=(i>>1);
        }
        DMODEIN(); //release DATA-line
        PULLUP1();
        SETSCK1; //clk #9 for ack
        S_PULSLONG;
        if (GETDATA()){ //check ack (DATA will be pulled down by SHT11)
                error=1;
        }
        S_PULSSHORT;
        SETSCK0;
        return(error); //error=1 in case of no acknowledge
}

// reads a byte form the Sensibus and gives an acknowledge in case of "ack=1"
// reversebits=1 caused the bits to be reversed (bit0=bit7, bit1=bit6,...)
uint8_t s_read_byte(uint8_t ack)
{
        uint8_t i=0x80;
        uint8_t val=0;
        DMODEIN(); //release DATA-line
        PULLUP1();
        while(i){ //shift bit for masking
                SETSCK1; //clk for SENSI-BUS
                S_PULSSHORT;
                if (GETDATA()){
                        val=(val | i); //read bit
                }
                SETSCK0;
                S_PULSSHORT;
                i=(i>>1);
        }
        DMODEOU(); 
        if (ack){
                //in case of "ack==1" pull down DATA-Line
                SETDAT0();
        }else{
                SETDAT1();
        }
        SETSCK1; //clk #9 for ack
        S_PULSLONG;
        SETSCK0;
        S_PULSSHORT;
        DMODEIN(); //release DATA-line
        PULLUP1();
        return (val);
}

// generates a sensirion specific transmission start
// This is the point where sensirion is not I2C standard conform and the
// main reason why the AVR TWI hardware support can not be used.
//       _____         ________
// DATA:      |_______|
//           ___     ___
// SCK : ___|   |___|   |______
void s_transstart()
{
        //Initial state
        SCKOUTP;
        SETSCK0;
        DMODEOU(); 
        SETDAT1();
        //
        S_PULSSHORT;
        SETSCK1;
        S_PULSSHORT;
        SETDAT0();
        S_PULSSHORT;
        SETSCK0;
        S_PULSLONG;
        SETSCK1;
        S_PULSSHORT;
        SETDAT1();
        S_PULSSHORT;
        SETSCK0;
        S_PULSSHORT;
        //
        DMODEIN(); //release DATA-line
        PULLUP1();
}

// communication reset: DATA-line=1 and at least 9 SCK cycles followed by transstart
//      _____________________________________________________         ________
// DATA:                                                     |_______|
//          _    _    _    _    _    _    _    _    _        ___    ___
// SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|  |___|   |______
void s_connectionreset()
{
        uint8_t i;
        //Initial state
        SCKOUTP;
        SETSCK0;
        DMODEOU(); 
        SETDAT1();
        for(i=0;i<9;i++){ //9 SCK cycles
                SETSCK1;
                S_PULSLONG;
                SETSCK0;
                S_PULSLONG;
        }
        s_transstart(); //transmission start
}

// resets the sensor by a softreset
char s_softreset()
{
        s_connectionreset(); //reset communication
        //send RESET-command to sensor:
        return (s_write_byte(RESET)); //return=1 in case of no response form the sensor
}


// makes a measurement (humidity/temperature) 
// return value: 1=write error, 0=ok wait for data
// if it returns OK then you must  call s_readmeasurement immediately 
// afterwards for the same sensor (sck pin must not change)
char s_measure(uint8_t mode)
{
        s_transstart(); //transmission start
        gSensor2mode=mode;
        if (s_write_byte(mode)){
                return(1);
        }
        return(0);
}

// return 1 if previously started measurement is ready
//
// Note that you can not wait too long. It takes between
// 60ms to 220ms for the measurement to complete and the
// sht11 pulls unfortunately the line only to gnd for a
// moment.  You should pull this function more or less
// constantly after the start of the measurement.
uint8_t s_resultready()
{
        if (GETDATA()!=0){
                return(0);
        }
        return(1);
}

// read previously initiated measurement 
// p_value returns 2 bytes
// mode: 1=humidity  0=temperature
// return value: 2=crc error
//
char s_readmeasurement(unsigned int *p_value)
{
        uint8_t msb,lsb;
        uint8_t checksum;
        uint8_t crc_state=0; 
        // the crc8 is computed over the entire communication from command to response data:
        crc_state=computeCRC8(bitswapbyte(gSensor2mode),crc_state);
        msb=s_read_byte(1); //read the first byte (MSB)
        crc_state=computeCRC8(bitswapbyte(msb),crc_state);
        lsb=s_read_byte(1); //read the second byte (LSB)
        *p_value=(msb<<8)|(lsb);
        crc_state=computeCRC8(bitswapbyte(lsb),crc_state);
        checksum =s_read_byte(0); //read checksum
        if (crc_state != checksum ) {
                return(2);
        }
        return(0);
}

// calculates temperature [C] 
// input : temp [Ticks] (14 bit)
// output: temp [C] times 10 (e.g 253 = 25.3'C)
// Sensor supply voltage: about 3.3V
//
int calc_sht11_temp(unsigned int t)
{
        //t = 10*(t *0.01 - 39.7);
        //or
        //t = t *0.1 - 397;
        t=t*0.1-825.7;
        return(t);
}

// calculates humidity [%RH]
// The relative humitidy is: -0.0000028*s*s + 0.0405*s - 4.0
// but this is very difficult to compute with integer math. 
// We use a simpler approach. See sht10_Non-Linearity_Compensation_Humidity_Sensors_E.pdf
//
// output: humi [%RH] (=integer value from 0 to 100)
uint8_t rhcalc_int(unsigned int s)
{
	// s is in the range from 100 to 3340
	unsigned int rh;
	//for s less than 1712: (143*s - 8192)/4096
	//for s greater than 1712: (111*s + 46288)/4096
	//s range: 100<s<3350
        //
	//rh = rel humi * 10
	if (s<1712){
                // div by 4:
		rh=(36*s - 2048)/102;
	}else{
                // div by 8:
		rh=(14*s + 5790)/51;
	}
	// round up as we will cut the last digit to full percent
	rh+=5;
	rh/=10;
        //
	if (rh>98){
		rh=100;
	}
	return((uint8_t)(rh));
}

// calculates humidity [%RH] with temperature compensation
// input : humi [Ticks] (12 bit), temperature in 'C * 100 (e.g 253 for 25.3'C)
// output: humi [%RH] (=integer value from 0 to 100)
uint8_t calc_sht11_humi(unsigned int h, int t)
{
        int rh;
        rh=rhcalc_int(h);
        // now calc. Temperature compensated humidity [%RH]
        // the correct formula is:
        // rh_true=(t/10-25)*(0.01+0.00008*(sensor_val))+rh; 
        // sensor_val ~= rh*30 
        // we use:
        // rh_true=(t/10-25) * 1/8;
        rh=rh + (t/80 - 3);
        return((uint8_t)rh);
}

// this is an approximation of 100*log10(x) and does not need the math
// library. The error is less than 5% in most cases.
// compared to the real log10 function for 2<x<100.
// input: x=2<x<100 
// output: 100*log10(x) 
// Idea by Guido Socher
int log10_approx(uint8_t x)
{
	int l,log;
	if (x==1){
		return(0);
	}
	if (x<8){
		return(11*x+11);
	}
	//
	log=980-980/x;
	log/=10;
	if (x<31){
		l=19*x;
		l=l/10;
		log+=l-4;
	}else{
		l=67*x;
		l=l/100;
		if (x>51 && x<81){
			log+=l +42;
		}else{
			log+=l +39;
		}
	}
	if (log>200) log=200;
	return(log);
}

// calculates dew point
// input: humidity [in %RH], temperature [in C times 10]
// output: dew point [in C times 10]
int calc_dewpoint(uint8_t rh,int t)
{ 
        // we use integer math and everything times 100
        int k,tmp;
        k = (100*log10_approx(rh)-20000)/43;
	// we do the calculations in many steps otherwise the compiler will try
	// to optimize and that creates nonsence as the numbers
	// can get too big or too small.
        tmp=t/10;
        tmp=881*tmp;
        tmp=tmp/(243+t/10);
        k+=tmp*2;
        tmp=1762-k;
        tmp=24310/tmp;
        tmp*=k;
        // dew point temp rounded:
	if (tmp<0){
		tmp-=51;
	}else{
		tmp+=51;
	}
        return (tmp/10);
}
uint8_t sht11_measure(int* mea_value, uint8_t code)
{
uint8_t error_code=1;
static unsigned int raw_val;
int temp_val;
uint8_t count=0;
uint8_t overtime=0;
s_connectionreset();	
if(s_measure(code)!=0)
		{
		//measure error
		error_code=2;
		}
	else
		{
		// measure ok
		//init_cnt2();
		//sei();
		//wait for result
		while ((s_resultready()==0) && (overtime==0))
		{
		count++;
		//_delay_ms(1);
		clock_delay_msec(1);
		if (count>220)
		overtime=1;
		}
		if (overtime==1)
		{
		//ovetime
		//error
		error_code=3;
		}
		else
		{
		//result ready
		//read
		if(s_readmeasurement(&raw_val)!=0)
		{
		//read error
		error_code=4;
		}
		else
		{
		//read ok
		if(code==MEASURE_TEMP)
		*mea_value=calc_sht11_temp(raw_val);
		else
		{
		sht11_measure(&temp_val, MEASURE_TEMP);
		*mea_value=calc_sht11_humi(raw_val,temp_val);
		}		
		//*mea_value=10;
		error_code=0;
		}

		}
	

		}
	
	
return (error_code);
}

uint8_t sht11_temp(int *temp_value)
{
uint8_t error_code;
error_code=sht11_measure(temp_value,MEASURE_TEMP); 
return (error_code);
}

uint8_t sht11_humidity(int *humi_value)
{
uint8_t error_code;
error_code=sht11_measure(humi_value,MEASURE_HUMI);
return (error_code);
}

// --- end of sensirion_protocol.c

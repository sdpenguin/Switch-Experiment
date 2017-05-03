#include "mbed.h"
#include "Adafruit_SSD1306.h"

//Switch input definition
#define SW_PIN p22

//Sampling period for the switch oscillator (us)
#define SW_PERIOD 20000

//Display interface pin definitions
#define D_MOSI_PIN p5
#define D_CLK_PIN p7
#define D_DC_PIN p8
#define D_RST_PIN p9
#define D_CS_PIN p10

//an SPI sub-class that sets up format and clock speed
class SPIPreInit:public SPI {
 public:
	SPIPreInit(PinName mosi, PinName miso, PinName clk):SPI(mosi, miso, clk) {
		format(8, 3);
		frequency(2000000);
	};
};

//Interrupt Service Routine prototypes (functions defined below)
void sedge();
void tout();

//Output for the alive LED
DigitalOut alive(LED1);

//External interrupt input from the switch oscillator
InterruptIn swin(SW_PIN);

//Switch sampling timer
Ticker swtimer;

//Registers for the switch counter, switch counter latch register and update flag
volatile uint16_t scounter = 0;
volatile uint16_t scount = 0;
volatile uint16_t update = 0;

//Initialise SPI instance for communication with the display
SPIPreInit gSpi(D_MOSI_PIN, NC, D_CLK_PIN);	//MOSI,MISO,CLK

//Initialise display driver instance
Adafruit_SSD1306_Spi gOled1(gSpi, D_DC_PIN, D_RST_PIN, D_CS_PIN, 64, 128);	//SPI,DC,RST,CS,Height,Width

int main()
{
	//Initialisation
	gOled1.setRotation(2);	//Set display rotation

	//Attach switch oscillator counter ISR to the switch input instance for a rising edge
	swin.rise(&sedge);

	//Attach switch sampling timer ISR to the timer instance with the required period
	swtimer.attach_us(&tout, SW_PERIOD);

	//Write some sample text
	gOled1.printf("%ux%u OLED Display\r\n", gOled1.width(),
		      gOled1.height());

	//Main loop
	while (1) {
		//Has the update flag been set?       
		if (update) {
			//Clear the update flag
			update = 0;

			//Set text cursor
			gOled1.setTextCursor(0, 0);

			//Write the latest switch osciallor count
			gOled1.printf("\n%05u  ", scount);

			//Copy the display buffer to the display
			gOled1.display();

			//Toggle the alive LED
			alive = !alive;
		}

	}
}

//Interrupt Service Routine for rising edge on the switch oscillator input
void sedge()
{
	//Increment the edge counter
	scounter++;
}

//Interrupt Service Routine for the switch sampling timer
void tout()
{
	//Read the edge counter into the output register
	scount = scounter;
	//Reset the edge counter
	scounter = 0;
	//Trigger a display update in the main loop
	update = 1;
}

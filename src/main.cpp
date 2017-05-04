#include "mbed.h"
#include "spi_init.h"
#include "counter.h"
#include "Adafruit_SSD1306.h"

#define SW_PINS p25, p24, p24, p22
#define SW_PERIOD 20000 // 20ms

void tout(void);

// Onboard LED 
DigitalOut alive(LED1);

PinName switch_pin[] = { SW_PINS };

Counter *switch_position[4];
Ticker  timer;

volatile uint16_t switch_count[4] = { 0, 0, 0, 0 };
volatile uint16_t switch_pressed[4] = { 0, 0, 0, 0 };
volatile uint16_t last_pressed[4] = { 0, 0, 0, 0 };

uint16_t current_f[4] = { 0, 0, 0, 0 };

volatile uint16_t update = 0;
uint16_t max_count[4];

// Initialise display
SPInit gSpi(D_MOSI_PIN, NC, D_CLK_PIN);	
Adafruit_SSD1306_Spi oled(gSpi, D_DC_PIN, D_RST_PIN, D_CS_PIN, 64, 128);

int main(void)
{
	oled.setRotation(2);
	wait(0.5);

	// Enable ISR for the switch rising edge
	for (int i = 0; i < 4; ++i) {
		switch_position[i] = new Counter(switch_pin[i]);	
	}

	//Attach switch sampling timer ISR to the timer instance with the required period
	timer.attach_us(&tout, SW_PERIOD);

	oled.clearDisplay();
	oled.printf("%ux%u Group Ay08-04\r\n", oled.width(), oled.height());

	for (;;) {
		if (update) {
			update = 0;

			oled.setTextCursor(0, 0);

			int frequency = 0;
			//Write the latest switch osciallor count
			for (int i = 3; i >= 0; --i) {
				current_f[i] += (switch_pressed[i] && !last_pressed[i]);
				if (current_f[i] > 9)
					current_f[i] = 0;
				
				frequency += current_f[i] * (10^i);
				oled.printf("\nS:%u C:%05u N:%u", switch_pressed[i], switch_count[i], current_f[i]);
			}

			oled.printf("\nF:%u F:%d", frequency, frequency);

			//Copy the display buffer to the display
			oled.display();

			alive = !alive;
		}

	}
}


//Interrupt Service Routine for the switch sampling timer
void tout(void)
{
	// Query how many times switch triggered
	for (int i = 0; i < 4; ++i) {
		switch_count[i] = switch_position[i]->read();
		switch_position[i]->write(0);
		
		last_pressed[i] = switch_pressed[i];

		if (max_count[i] < switch_count[i])
			max_count[i] = switch_count[i];

		if (switch_count[i] < (3*max_count[i])/5)
			switch_pressed[i] = 1;
		else if (switch_count[i] > (4*max_count[i])/5)
			switch_pressed[i] = 0;
	}
	// Update display
	update = 1;
}

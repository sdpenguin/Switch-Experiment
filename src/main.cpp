#include "mbed.h"

#include "Adafruit_SSD1306.h"
#include "counter.h"
#include "spi_init.h"

#define SW_PINS p24, p23, p22, p21
#define SW_PERIOD 20000 // 20ms
#define ARRAY_SIZE 360

void tout(void);
void square(void);
void triangle(void);
void sine(void);

AnalogOut out_wave(p18);
PinName switch_pin[] = {SW_PINS};

Counter *switch_position[4];
Ticker timer;
Ticker pwm;

volatile uint16_t switch_count[4] = {0, 0, 0, 0};
volatile uint16_t switch_pressed[4] = {0, 0, 0, 0};
volatile uint16_t last_pressed[4] = {0, 0, 0, 0};

uint16_t current_f[4] = {0, 0, 0, 0};

volatile uint16_t update = 0;
volatile uint16_t amp = 0;

const double pi = 3.141592653589793238462;
const double offset = 65535 / 2; // Offset is 1/2 the total bits
double rads = 0.0;
uint16_t sample = 0;

uint16_t sineArray[ARRAY_SIZE];

// Initialise display
SPInit gSpi(D_MOSI_PIN, NC, D_CLK_PIN);
Adafruit_SSD1306_Spi oled(gSpi, D_DC_PIN, D_RST_PIN, D_CS_PIN, 64, 128);

int main(void)
{
	for (int i = 0; i < ARRAY_SIZE; i++) {
		int rads = (pi * i) / (ARRAY_SIZE / 2.0f);
		sineArray[i] =
		    (uint16_t)(0.5f * (offset * (cos(rads + pi))) + offset);
	}

	uint16_t frequency = 0;
	uint16_t old_frequency = 0;

	out_wave.write(0.5);
	oled.setRotation(2);
	wait(0.5);

	// Enable ISR for the switch rising edge
	for (int i = 0; i < 4; ++i) {
		switch_position[i] = new Counter(switch_pin[i]);
	}

	// Attach switch sampling timer ISR to the timer instance with the
	// required period
	timer.attach_us(&tout, SW_PERIOD);

	oled.clearDisplay();
	oled.printf("%ux%u Group Ay08-04\r\n", oled.width(), oled.height());

	for (;;) {
		if (update) {
			update = 0;

			oled.setTextCursor(0, 0);

			// Write the latest switch osciallor count
			for (int i = 3; i >= 0; --i) {
				current_f[i] += (switch_pressed[i] && !last_pressed[i]);
				if (current_f[i] > 9)
					current_f[i] = 0;
				oled.printf("\nS:%u C:%05u N:%u", switch_pressed[i], switch_count[i], current_f[i]);
			}

			old_frequency = frequency;
			frequency = 1000 * current_f[3] + 100 * current_f[2] + 10 * current_f[1] + current_f[0];

			if (frequency != old_frequency) {

				oled.printf("\nF:%u   ", frequency);

				if (frequency)
					pwm.attach_us( &sine, 1000000 / (frequency * 360));
			}
			// Copy the display buffer to the display
			oled.display();
		}
	}
}

// Interrupt Service Routine for the switch sampling timer
void tout(void)
{
	// Query how many times switch triggered
	for (int i = 0; i < 4; ++i) {
		switch_count[i] = switch_position[i]->read();
		switch_position[i]->write(0);

		last_pressed[i] = switch_pressed[i];

		if (switch_count[i] < 600)
			switch_pressed[i] = 1;
		else if (switch_count[i] > 700)
			switch_pressed[i] = 0;
	}
	// Update display
	update = 1;
}

/*
void square(void){
		out_wave = !out_wave;
}
*/
void triangle(void)
{
	amp++;
	out_wave = (float)amp / 256;
	if (amp == 256)
		amp = 0;
}

void sine(void)
{
	if (amp == ARRAY_SIZE)
		amp = 0;
	out_wave.write_u16(sineArray[amp]);
	amp++;
}

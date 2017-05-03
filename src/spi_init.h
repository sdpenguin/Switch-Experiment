#define D_MOSI_PIN p5
#define D_CLK_PIN p7
#define D_DC_PIN p8
#define D_RST_PIN p9
#define D_CS_PIN p10

//an SPI sub-class that sets up format and clock speed
class SPInit:public SPI {
public:
	SPInit(PinName mosi, PinName miso, PinName clk):SPI(mosi, miso, clk) {
		format(8, 3);
		frequency(2000000);
	};
};

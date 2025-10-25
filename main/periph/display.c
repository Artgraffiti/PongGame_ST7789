#include "display.h"

void init_display(TFT_t *dev) {
  // Change SPI Clock Frequency
  spi_clock_speed(40000000);  // 40MHz
  // spi_clock_speed(60000000);  // 60MHz

  spi_master_init(dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO, CONFIG_BL_GPIO);
  lcdInit(dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);
}
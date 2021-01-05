/*
	Copyright 2017 - 2021 Benjamin Vedder	benjamin@vedder.se

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "deca_port.h"

#include "nordic_common.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_gpiote.h"

#include <string.h>

/*
 * Connections
 *
 * P0.19: DW_IRQ
 * P0.16: DW_SCK
 * P0.20: DW_MOSI
 * P0.18: DW_MISO
 * P0.17: DW_SPI_CS
 * P0.24: DW_RST
 *
 */

#define DW_PIN_IRQ		NRF_GPIO_PIN_MAP(0, 19)
#define DW_PIN_SCK		NRF_GPIO_PIN_MAP(0, 16)
#define DW_PIN_MOSI		NRF_GPIO_PIN_MAP(0, 20)
#define DW_PIN_MISO		NRF_GPIO_PIN_MAP(0, 18)
#define DW_PIN_CS		NRF_GPIO_PIN_MAP(0, 17)
#define DW_PIN_RST		NRF_GPIO_PIN_MAP(0, 24)

#define NRF_DRV_SPI_DEFAULT_CONFIG_2M                       \
{                                                            \
    .sck_pin      = DW_PIN_SCK,      \
    .mosi_pin     = DW_PIN_MOSI,     \
    .miso_pin     = DW_PIN_MISO,     \
    .ss_pin       = DW_PIN_CS,                \
    .irq_priority = APP_IRQ_PRIORITY_LOW, \
    .orc          = 0xFF,                                    \
    .frequency    = NRF_DRV_SPI_FREQ_2M,                     \
    .mode         = NRF_DRV_SPI_MODE_0,                      \
    .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,         \
}


#define NRF_DRV_SPI_DEFAULT_CONFIG_8M                       \
{                                                            \
    .sck_pin      = DW_PIN_SCK,      \
    .mosi_pin     = DW_PIN_MOSI,     \
    .miso_pin     = DW_PIN_MISO,     \
    .ss_pin       = DW_PIN_CS,                \
    .irq_priority = APP_IRQ_PRIORITY_LOW, \
    .orc          = 0xFF,                                    \
    .frequency    = NRF_DRV_SPI_FREQ_8M,                     \
    .mode         = NRF_DRV_SPI_MODE_0,                      \
    .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,         \
}

// Private variables
static volatile bool isr_enabled = true;
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE_1;
static volatile bool spi_xfer_done;

// Private functions
static void spi_event_handler(nrf_drv_spi_evt_t const * p_event, void * p_context) {
	spi_xfer_done = true;
}

//static void irq_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
//	(void)action;
//
//	if (!isr_enabled) {
//		return;
//	}
//
//	while (nrf_gpio_pin_read(DW_PIN_IRQ)) {
//		dwt_isr();
//	}
//}

void deca_port_process_irq(void) {
	if (!isr_enabled) {
		return;
	}

	while (nrf_gpio_pin_read(DW_PIN_IRQ)) {
		dwt_isr();
	}
}

void deca_port_init(void) {
	nrf_gpio_cfg_output(DW_PIN_RST);
	nrf_gpio_cfg_output(DW_PIN_CS);
	nrf_gpio_cfg_input(DW_PIN_IRQ, NRF_GPIO_PIN_NOPULL);

//	nrf_drv_gpiote_init();
//	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
//	in_config.pull = NRF_GPIO_PIN_PULLDOWN;
//	nrf_drv_gpiote_in_init(DW_PIN_IRQ, &in_config, irq_handler);
//	nrf_drv_gpiote_in_event_enable(DW_PIN_IRQ, true);

	// Reset and wake up
	deca_port_reset();
	deca_port_set_spi_slow();
	nrf_gpio_pin_clear(DW_PIN_CS);
	nrf_delay_ms(600);
	nrf_gpio_pin_set(DW_PIN_CS);
}

void deca_port_reset(void) {
	nrf_gpio_cfg_output(DW_PIN_RST);
	nrf_gpio_pin_clear(DW_PIN_RST);
	nrf_delay_ms(2);
	nrf_gpio_cfg_input(DW_PIN_RST, NRF_GPIO_PIN_NOPULL);
	nrf_delay_ms(2);
}

void deca_port_go_to_sleep(void) {
	dwt_configuresleep(DWT_PRESRV_SLEEP, DWT_SLP_EN | DWT_WAKE_CS);
	dwt_entersleep();
//	nrf_gpio_cfg_default(DW_PIN_IRQ);
//	nrf_gpio_cfg_default(DW_PIN_SCK);
//	nrf_gpio_cfg_default(DW_PIN_MOSI);
//	nrf_gpio_cfg_default(DW_PIN_MISO);
//	nrf_gpio_cfg_default(DW_PIN_CS);
//	nrf_gpio_cfg_default(DW_PIN_RST);
}

void deca_port_set_spi_slow(void) {
	nrf_drv_spi_uninit(&spi);
	nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG_2M;
	nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL);
	nrf_delay_ms(2);
}

void deca_port_set_spi_fast(void) {
	nrf_drv_spi_uninit(&spi);
	nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG_8M;
	nrf_drv_spi_init(&spi, &spi_config, spi_event_handler,NULL);
	nrf_delay_ms(2);
}

int deca_port_writetospi(uint16 headerLength, const uint8 *headerBuffer, uint32 bodylength, const uint8 *bodyBuffer) {
	uint8 *p1;
	uint32 idatalength = 0;

	idatalength = headerLength + bodylength;

	uint8 idatabuf[idatalength];
	uint8 itempbuf[idatalength];

	memset(idatabuf, 0, idatalength);
	memset(itempbuf, 0, idatalength);

	p1 = idatabuf;
	memcpy(p1, headerBuffer, headerLength);
	p1 += headerLength;
	memcpy(p1, bodyBuffer, bodylength);

	spi_xfer_done = false;
	nrf_drv_spi_transfer(&spi, idatabuf, idatalength, itempbuf, idatalength);
	while(!spi_xfer_done) {}

	return 0;
}

int deca_port_readfromspi(uint16 headerLength, const uint8 *headerBuffer, uint32 readlength, uint8 *readBuffer) {
	uint8 *p1;
	uint32 idatalength = 0;

	idatalength = headerLength + readlength;

	uint8 idatabuf[idatalength];
	uint8 itempbuf[idatalength];

	memset(idatabuf, 0, idatalength);
	memset(itempbuf, 0, idatalength);

	p1 = idatabuf;
	memcpy(p1, headerBuffer, headerLength);

	p1 += headerLength;
	memset(p1, 0x00, readlength);

	spi_xfer_done = false;
	nrf_drv_spi_transfer(&spi, idatabuf, idatalength, itempbuf, idatalength);
	while(!spi_xfer_done) {}

	p1 = itempbuf + headerLength;

	memcpy(readBuffer, p1, readlength);

	return 0;
}

void deca_port_sleep(unsigned int time_ms) {
	nrf_delay_ms(time_ms);
}

decaIrqStatus_t decamutexon(void) {
	decaIrqStatus_t ret = isr_enabled;
	isr_enabled = false;
	return ret;
}

void decamutexoff(decaIrqStatus_t s) {
	if (s) {
		isr_enabled = true;
	}
}

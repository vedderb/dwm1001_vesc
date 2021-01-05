/*
	Copyright 2019 Benjamin Vedder	benjamin@vedder.se

	This file is part of the VESC firmware.

	The VESC firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The VESC firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#ifndef I2C_BB_H_
#define I2C_BB_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
	int sda_pin;
	int scl_pin;
	bool has_started;
	bool has_error;
} i2c_bb_state;

void i2c_bb_init(i2c_bb_state *s);
void i2c_bb_restore_bus(i2c_bb_state *s);
bool i2c_bb_tx_rx(i2c_bb_state *s, uint16_t addr, uint8_t *txbuf, size_t txbytes, uint8_t *rxbuf, size_t rxbytes);

#endif /* I2C_BB_H_ */

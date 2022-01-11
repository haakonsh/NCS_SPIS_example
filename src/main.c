/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <logging/log.h>
#include <nrfx.h>
#include <nrfx_spis.h>

LOG_MODULE_REGISTER(app);

#define BUF_SIZE 	64

#define PIN_SCK 	28
#define PIN_CS 		31
#define PIN_MOSI 	29
#define PIN_MISO 	30

nrfx_spis_t spis = NRFX_SPIS_INSTANCE(0);

static K_MEM_SLAB_DEFINE(spis_slab, BUF_SIZE, 4, 4);
uint8_t *tx_buf1, *tx_buf2, *rx_buf1, *rx_buf2;

void spis_evt_handler(nrfx_spis_evt_t const * p_event, void * p_context)
{
	nrfx_err_t err = NRFX_SUCCESS;

	switch(p_event->evt_type)
	{
		case NRFX_SPIS_BUFFERS_SET_DONE:
			printk("Buffers are set and the SPIS is ready for a transfer\n");

			break;

		case NRFX_SPIS_XFER_DONE:
			memcpy(tx_buf2, tx_buf1, p_event->tx_amount);			
			memcpy(rx_buf2, rx_buf1, p_event->tx_amount);
		
			printk("SPIS transfer done! Sent %dbytes and received %d bytes.\n TX:", p_event->tx_amount, p_event->rx_amount);
			printk("\nRX: %s\n", rx_buf1);
			printk("\nTX: %s\n", tx_buf1);

			err = nrfx_spis_buffers_set(&spis, tx_buf1, BUF_SIZE, rx_buf1, BUF_SIZE);
			__ASSERT(err == 0, "Failed to prepare spis buffers");
			break;

		case NRFX_SPIS_EVT_TYPE_MAX:
			printk("Oops! Enumeration upper bound!\n");
			break;
		
		default:
			printk("Oops! Invalid operation!\n");
			break;
	}
}

void init_spis(void)
{
	nrfx_err_t err = NRFX_SUCCESS;

	nrfx_spis_config_t cfg = NRFX_SPIS_DEFAULT_CONFIG(PIN_SCK, PIN_MOSI, PIN_MISO, PIN_CS);

	err = nrfx_spis_init(&spis, &cfg, spis_evt_handler, NULL);
	__ASSERT(err == 0, "Failed to initialize the spis driver. Error: %d\n", err)	

	err = k_mem_slab_alloc(&spis_slab, (void **)&tx_buf1, K_NO_WAIT);
	__ASSERT(err == 0, "Failed to alloc slab\n");

	err = k_mem_slab_alloc(&spis_slab, (void **)&tx_buf2, K_NO_WAIT);
	__ASSERT(err == 0, "Failed to alloc slab\n");

	err = k_mem_slab_alloc(&spis_slab, (void **)&rx_buf1, K_NO_WAIT);
	__ASSERT(err == 0, "Failed to alloc slab\n");

	err = k_mem_slab_alloc(&spis_slab, (void **)&rx_buf2, K_NO_WAIT);
	__ASSERT(err == 0, "Failed to alloc slab\n");

	err = nrfx_spis_buffers_set(&spis, tx_buf1, BUF_SIZE, rx_buf1, BUF_SIZE);
	__ASSERT(err == 0, "Failed to prepare spis buffers\n");
}

void main(void)
{
	/* Connect SPIS_0 IRQ to nrfx_spis_0_irq_handler */
	IRQ_CONNECT(SPIM0_SPIS0_SPI0_IRQn, IRQ_PRIO_LOWEST, nrfx_isr, nrfx_spis_0_irq_handler, 0);
	
	init_spis();
	printk("Initialized the SPIS peripheral!\n");

	strcpy(tx_buf1, "Hello!\n");

	while(1)
	{
		k_msleep(1000);
		//printk("Alive!\n");
	}
}

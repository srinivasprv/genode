/*
 * \brief  I.MX6-specific implementation of the Block::Driver interface
 * \brief  Adapted from Omap4 sd_card driver
 * \author Praveen Srinivas (IIT Madras, India)
 * \author Norman Feske
 * \date   2015-02-10
 */

/*
 * Copyright (C) 2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <util/mmio.h>
#include <os/attached_io_mem_dataspace.h>
#include <base/printf.h>
#include <timer_session/connection.h>
#include <block/component.h>

/* local includes */
#include <usdhc.h>

namespace Block {
	using namespace Genode;
	class Imx6_driver;
}


class Block::Imx6_driver : public Block::Driver
{
	private:

		struct Timer_delayer : Timer::Connection, Mmio::Delayer
		{
			/**
			 * Implementation of 'Delayer' interface
			 */
			void usleep(unsigned us)
			{
				/* polling */
				if (us == 0)
					return;

				Timer::Connection::usleep(us);
			}
		} _delayer;

		enum {
			BASE = Genode::Board_base::USDHC4_MMIO_BASE, /* host controller base for eMMC */
			SIZE = Genode::Board_base::USDHC4_MMIO_SIZE,
		};


		/* display sub system registers */
		Attached_io_mem_dataspace _mmio;

		/* mobile storage host controller instance */
		Imx6_usdhc_controller _controller;

		bool const _use_dma;

	public:

		Imx6_driver(bool use_dma)
		:
			_mmio(BASE, SIZE),
			_controller((addr_t)_mmio.local_addr<void>(),
			            _delayer, use_dma),
			_use_dma(use_dma)
		{
			Sd_card::Card_info const card_info = _controller.card_info();

			PLOG("SD/MMC card detected");
			PLOG("capacity: %zd MiB", card_info.capacity_mb());
		}


		/*****************************
		 ** Block::Driver interface **
		 *****************************/

		Genode::size_t block_size() { return 512; }

		virtual Block::sector_t block_count()
		{
			return _controller.card_info().capacity_mb() * 1024 * 2;
		}

		Block::Session::Operations ops()
		{
			Block::Session::Operations o;
			o.set_operation(Block::Packet_descriptor::READ);
			o.set_operation(Block::Packet_descriptor::WRITE);
			return o;
		}

		void read(Block::sector_t    block_number,
		          Genode::size_t     block_count,
		          char              *out_buffer,
		          Packet_descriptor &packet)
		{
			if (!_controller.read_blocks(block_number, block_count, out_buffer))
				throw Io_error();
			ack_packet(packet);
		}

		void write(Block::sector_t    block_number,
		           Genode::size_t     block_count,
		           char const        *buffer,
		           Packet_descriptor &packet)
		{
			if (!_controller.write_blocks(block_number, block_count, buffer))
				throw Io_error();
			ack_packet(packet);
		}

		void read_dma(Block::sector_t   block_number,
		              Genode::size_t    block_count,
		              Genode::addr_t    phys,
		              Packet_descriptor &packet)
		{
			if (!_controller.read_blocks_dma(block_number, block_count, phys))
				throw Io_error();
			ack_packet(packet);
		}

		void write_dma(Block::sector_t    block_number,
		               Genode::size_t     block_count,
		               Genode::addr_t     phys,
		               Packet_descriptor &packet)
		{
			if (!_controller.write_blocks_dma(block_number, block_count, phys))
				throw Io_error();
			ack_packet(packet);
		}

		bool dma_enabled() { return _use_dma; }

		Genode::Ram_dataspace_capability alloc_dma_buffer(Genode::size_t size) {
			return Genode::env()->ram_session()->alloc(size, UNCACHED); }

		void free_dma_buffer(Genode::Ram_dataspace_capability c) {
			return Genode::env()->ram_session()->free(c); }
};

#endif /* _DRIVER_H_ */

/*
 * \brief  Freescale-i.MX6 controller registers
 * \brief  Adapted from Omap4 sd_card driver
 * \author Praveen Srinivas (IIT Madras, India)
 * \author Norman Feske
 * \date   2015-03-12
 */

/*
 * Copyright (C) 2012-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _USDHC_H_
#define _USDHC_H_

/* Genode includes */
#include <util/mmio.h>
#include <os/attached_ram_dataspace.h>
#include <irq_session/connection.h>
#include <drivers/board_base.h>

/* local includes */
#include <sd_card.h>

struct Usdhc : Genode::Mmio
{
	enum { verbose = false };

	typedef Genode::size_t size_t;

	Usdhc(Genode::addr_t const mmio_base) : Genode::Mmio(mmio_base) { }

	struct Dmaaddr : Register<0x00,32> { };

	struct Arg : Register<0x08,32> { };

	/**
	 * Command register
	 */
	struct Cmd : Register<0x0c, 32>
	{
		struct Index : Bitfield<24, 6> { };

		/**
		 * Data present
		 */
		struct Dp : Bitfield<21, 1> { };

		struct Rsp_type : Bitfield<16, 2>
		{
			enum Response { RESPONSE_NONE             = 0,
			                RESPONSE_136_BIT          = 1,
			                RESPONSE_48_BIT           = 2,
			                RESPONSE_48_BIT_WITH_BUSY = 3 };
		};

	};

	/**
	 * Transfer length configuration
	 */
	struct Blk : Register<0x04, 32>
	{
		struct Blen : Bitfield<0, 12> { };
		struct Nblk : Bitfield<16, 16> { };
	};

	/**
	 * Response bits 0..31
	 */
	struct Rsp0 : Register<0x10, 32> { };

	/**
	 * Response bits 32..63
	 */
	struct Rsp1 : Register<0x14, 32> { };

	/**
	 * Response bits 64..95
	 */
	struct Rsp2 : Register<0x18, 32> { };

	/**
	 * Response bits 96..127
	 */
	struct Rsp3 : Register<0x1c, 32> { };

	struct Data : Register<0x20, 32> { };

	struct Pstate : Register<0x24, 32>
	{
		/**
		 * Command inhibit
		 */
		struct Cmdi : Bitfield<0, 1> { };

		/**
		 * Data inhibit
		 */
		struct Dti : Bitfield<0, 1> { };

		/**
		 * Buffer write enable status
		 */
		struct Bwe : Bitfield<10, 1> { };

		/**
		 * Buffer read enable status
		 */
		struct Bre : Bitfield<11, 1> { };

		/**
		 * Internal Clock Stable
		 */
		struct Sdstb : Bitfield<3,1> { };
	};

	struct Hctl : Register<0x28, 32>
	{
		/**
		 * Wakeup event enable
		 */
		struct Iwe : Bitfield<24, 1> { };

		/**
		 * Endian mode
		 */
		struct Emode : Bitfield<4,2>
		{
			enum { BIG_END = 0, HALF_BIG = 1, LITTLE_END = 2 };
		};

		/**
		 * Data3 Card Detect
		 */
		struct D3cd : Bitfield<3,1> { };

		/**
		 * Data transfer width
		 */
		struct Dtw : Bitfield<1, 2>
		{
			enum { ONE_BIT = 0, FOUR_BITS = 1, EIGHT_BITS = 2, MASK = 3 };
		};

		/**
		 * DMA mode select
		 */
		struct Dmasel : Bitfield<8,2>
		{
			enum { SIMPLE = 0, ADMA1 = 1, ADMA2 = 2 };
		};
	};

	struct Sysctl : Register<0x2c, 32>
	{
		/**
		 * Internal clock enable
		 */
		struct Ice : Bitfield<0, 1> { };

		/**
		 * Clock Divider
		 */
		struct Clkd : Bitfield<4, 12> { };

		/**
		 * Software reset all
		 */
		struct Sra : Bitfield<24, 1> { };

		/**
		 * Software reset for command line
		 */
		struct Src : Bitfield<25, 1> { };

		/**
		 * Software reset for data
		 */
		struct Srd : Bitfield<26, 1> { };

		/**
		 * Data timeout counter
		 */
		struct Dto : Bitfield<16, 4>
		{
			enum { TCF_2_POW_27 = 0xe };
		};
	};

	/**
	 * Interrupt Status register
	*/
	struct Stat : Register<0x30, 32>
	{
		/**
		 * Transfer completed
		 */
		struct Tc : Bitfield<1, 1> { };

		/**
		 * Command completed
		 */
		struct Cc : Bitfield<0, 1> { };

		/**
		 * Command timed out
		 */
		struct Cto  : Bitfield<16, 1> { };
		
		/**
		 * Dma interrupt
		 */
		struct Dint : Bitfield<3,1> { };

		/**
		 *  AC12 error
		 */
		struct Ac12 : Bitfield<24,1> { };

		/**
		 * DMA error 
		 */
		struct Dmae : Bitfield<28,1> { };
	};

	/**
	 * Interrupt enable
	 */
	struct Ie : Register<0x34, 32>
	{
		/**
		 * Command completed
		 */
		struct Cc_enable : Bitfield<0, 1> { };

		/**
		 * Transfer completed
		 */
		struct Tc_enable : Bitfield<1, 1> { };

		/**
		 * Card interrupt
		 */
		struct Cirq_enable : Bitfield<8, 1> { };

		/**
		 * Command timeout error
		 */
		struct Cto_enable : Bitfield<16, 1> { };

		/**
		 * DMA Int enable
		 */
		struct Dma_enable : Bitfield<3, 1> { };

		/**
		 * DMA Error
		 */
		struct Dmae : Bitfield<28, 1> { };
	};

	struct Ise : Register<0x38, 32>
	{
		/*
		 * The naming follows the lines of the 'Ie' register
		 */
		struct Tc_sigen  : Bitfield<1, 1> { };
		struct Cto_sigen : Bitfield<16, 1> { };

		/**
		 * DMA Int enable
		 */
		struct Dma_sigen : Bitfield<3, 1> { };

		/**
		 * DMA Error
		 */
		struct Dmae_sigen : Bitfield<28, 1> { };
	};

	/**
	 * Capabilities
	 */
	struct Capa : Register<0x40, 32>
	{
		struct Vs30 : Bitfield<25, 1> { };
		struct Vs18 : Bitfield<26, 1> { };
	};

	/**
	 * Water Mark level
	 */
	struct Wmlvl : Register<0x44, 32>
	{
		/**
		 * Write Burst Length
		 */
		struct Wblen : Bitfield<24,5> { };

		/**
		 * Write Watermark Level
		 */
		struct Wlvl : Bitfield<16,8> { };

		/**
		 * Read Burst Length
		 */
		struct Rblen : Bitfield<8,5> { };

		/**
		 * Read Watermark Level
		 */
		struct Rlvl : Bitfield<0,8> { };
	};

	/**
	 * Mix control
	 */
	struct Mixctrl : Register<0x48, 32>
	{
		/**
		 * Data direction
		 */
		struct Ddir : Bitfield<4, 1>
		{
			enum { WRITE = 0, READ = 1 };
		};

		/**
		 * Block count enable
		 */
		struct Bce : Bitfield<1, 1> { };

		/**
		 * Multiple block select
		 */
		struct Msbs : Bitfield<5, 1> { };

		/**
		 * Auto-CMD12 enable
		 */
		struct Acen : Bitfield<2, 1> { };

		/**
		 * DMA enable
		 */
		struct De : Bitfield<0, 1> { };
	};

	/**
	 * ADMA error status
	 */
	struct Admaerr : Register<0x54,32>
	{
		/**
		 * length mismatch error
		 */
		struct Lenmm : Bitfield<2,1>{};
		
		/**
		 * descriptor error
		 */
		struct Desc : Bitfield<3,1>{};
	};

	/**
	 * ADMA system address
	 *
	 * Base address of the ADMA descriptor table
	 */
	struct Admasal : Register<0x58, 32> { };

	/**
	 * ADMA descriptor layout
	 */
	struct Adma_desc : Genode::Register<64>
	{
		struct Valid   : Bitfield<0, 1> { };
		struct Ent     : Bitfield<1, 1> { };
		struct Int     : Bitfield<2, 1> { };
		struct Act1    : Bitfield<4, 1> { };
		struct Act2    : Bitfield<5, 1> { };
		struct Length  : Bitfield<16, 16> { };
		struct Address : Bitfield<32, 32> { };
	};

	/**
	 * Vendor Specific Register
	 */
	struct Vspec : Register<0xc0, 32>
	{
		/**
		 * SD voltage select - <Hctl:Sdvs> in SDHCI
		 */
		struct Volt : Bitfield<1, 1> 
		{
			enum Voltage { VOLTAGE_1_8 = 0,
			               VOLTAGE_3_0 = 1 };
		};

		/**
		 * Clock enable force - <Sysctl:Ce> in SDHCI
		 */
		struct Fce : Bitfield<8,1> { };
	};

	bool reset_cmd_line(Delayer &delayer)
	{
		write<Sysctl::Src>(1);

		/*
		 * We must poll quickly. If we waited too long until checking the bit,
		 * the polling would be infinite. Apparently the hardware depends on
		 * the timing here.
		 */
		if (!wait_for<Sysctl::Src>(1, delayer, 1000, 0)) {
			PERR("reset of cmd line timed out (src != 1)");
			return false;
		}

		if (!wait_for<Sysctl::Src>(0, delayer, 1000, 0)) {
			PERR("reset of cmd line timed out (src != 0)");
			return false;
		}

		return true;
	}

	void disable_irq()
	{
		write<Ise>(0);
		write<Ie>(0);
		write<Stat>(~0);
	}

	enum Bus_width { BUS_WIDTH_1, BUS_WIDTH_4, BUS_WIDTH_8 };

	void bus_width(Bus_width bus_width)
	{
		switch (bus_width) {
		case BUS_WIDTH_1:
			write<Hctl::Dtw>(Hctl::Dtw::ONE_BIT);
			break;

		case BUS_WIDTH_4:
			write<Hctl::Dtw>(Hctl::Dtw::FOUR_BITS);
			break;

		case BUS_WIDTH_8:
			write<Hctl::Dtw>(Hctl::Dtw::EIGHT_BITS);
			break;
		}
	}

	void stop_clock()
	{
		write<Vspec::Fce>(0);
	}

	enum Clock_divider { CLOCK_DIV_0, CLOCK_DIV_240 };

	bool set_and_enable_clock(enum Clock_divider divider, Delayer &delayer)
	{
		write<Sysctl::Dto>(Sysctl::Dto::TCF_2_POW_27);

		switch (divider) {
		case CLOCK_DIV_0:   write<Sysctl::Clkd>(0);   break;
		case CLOCK_DIV_240: write<Sysctl::Clkd>(240); break;
		}

		write<Sysctl::Ice>(1);

		/* wait for clock to become stable */
		if (!wait_for<Pstate::Sdstb>(1, delayer)) {
			PERR("clock enable timed out");
			return false;
		}

		/* enable clock */
		write<Vspec::Fce>(1);

		return true;
	}

	enum Voltage { VOLTAGE_3_0, VOLTAGE_1_8 };

	void set_bus_power(Voltage voltage)
	{
		switch (voltage) {
		case VOLTAGE_3_0:
			write<Vspec::Volt>(Vspec::Volt::VOLTAGE_3_0);
			break;

		case VOLTAGE_1_8:
			write<Vspec::Volt>(Vspec::Volt::VOLTAGE_1_8);
			break;
		}

		write<Capa::Vs18>(1);

		if (voltage == VOLTAGE_3_0)
			write<Capa::Vs30>(1);
	}

	bool init_stream(Delayer &delayer)
	{
		write<Ie>(0x307f0033);

		write<Cmd>(0);

		if (!wait_for<Stat::Cc>(1, delayer, 1000*1000, 0)) {
			PERR("init stream timed out");
			return false;
		}

		write<Stat>(~0);
		read<Stat>();

		return true;
	}

};


struct Imx6_usdhc_controller : private Usdhc, public Sd_card::Host_controller
{
	private:

		Delayer           &_delayer;
		Sd_card::Card_info _card_info;
		bool const         _use_dma;

		/*
		 * DMA memory for holding the ADMA descriptor table
		 */
		enum { ADMA_DESC_MAX_ENTRIES = 1024 };
		Genode::Attached_ram_dataspace _adma_desc_ds;
		Adma_desc::access_t *    const _adma_desc;
		Genode::addr_t           const _adma_desc_phys;

		Genode::Irq_connection _irq;

		Sd_card::Card_info _init()
		{
			using namespace Sd_card;

			write<Hctl>(0x0);

			set_bus_power(VOLTAGE_3_0);

			disable_irq();

			bus_width(BUS_WIDTH_1);

			_delayer.usleep(10*1000);

			stop_clock();
			if (!set_and_enable_clock(CLOCK_DIV_240, _delayer)) {
				PERR("set_clock failed");
				throw Detection_failed();
			}

			if (!init_stream(_delayer)) {
				PERR("sending the initialization stream failed");
				throw Detection_failed();
			}

			write<Blk>(0);

			_delayer.usleep(1000);

			if (!issue_command(Go_idle_state())) {
				PWRN("Go_idle_state command failed");
				throw Detection_failed();
			}

			_delayer.usleep(2000);

			if (!issue_command(Send_if_cond())) {
				PWRN("Send_if_cond command failed");
				throw Detection_failed();
			}

			if (read<Rsp0>() != 0x1aa) {
				PERR("unexpected response of Send_if_cond command");
				throw Detection_failed();
			}

			/*
			 * We need to issue the same Sd_send_op_cond command multiple
			 * times. The first time, we receive the status information. On
			 * subsequent attempts, the response tells us that the card is
			 * busy. Usually, the command is issued twice. We give up if the
			 * card is not reaching busy state after one second.
			 */

			int i = 1000;
			for (; i > 0; --i) {
				if (!issue_command(Sd_send_op_cond(0x18000, true))) {
					PWRN("Sd_send_op_cond command failed");
					throw Detection_failed();
				}

				if (Ocr::Busy::get(read<Rsp0>()))
					break;

				_delayer.usleep(1000);
			}

			if (i == 0) {
				PERR("Sd_send_op_cond timed out, could no power-on SD card");
				throw Detection_failed();
			}

			Card_info card_info = _detect();

			/*
			 * Switch card to use 4 data signals
			 */
			if (!issue_command(Set_bus_width(Set_bus_width::Arg::Bus_width::FOUR_BITS),
			                   card_info.rca())) {
				PWRN("Set_bus_width(FOUR_BITS) command failed");
				throw Detection_failed();
			}

			bus_width(BUS_WIDTH_4);

			_delayer.usleep(10*1000);

			stop_clock();
			if (!set_and_enable_clock(CLOCK_DIV_0, _delayer)) {
				PERR("set_clock failed");
				throw Detection_failed();
			}

			/* enable IRQs */
			write<Ie::Tc_enable>(1);
			write<Ie::Cto_enable>(1);
			write<Ise::Tc_sigen>(1);
			write<Ise::Cto_sigen>(1);

			/**
			 * Water mark level should be < 128, for both read/write.
			 * Burst length should be less than or equal to Water mark Level.
			 * Values 0x10,0x8 are taken from Linux driver implementation.
			 */
			write<Wmlvl::Wblen>(0x8);
			write<Wmlvl::Wlvl>(0x10);
			write<Wmlvl::Rblen>(0x8);
			write<Wmlvl::Rlvl>(0x10);

			/* endianness - little */
			write<Hctl::Emode>(Hctl::Emode::LITTLE_END);

			return card_info;
		}

		/**
		 * Marshal ADMA descriptors according to block request
		 *
		 * \return false if block request is too large
		 */
		bool _setup_adma_descriptor_table(size_t block_count,
		                                  Genode::addr_t out_buffer_phys)
		{
			using namespace Sd_card;

			/* reset ADMA offset to first descriptor */
			write<Admasal>(_adma_desc_phys);

			enum { BLOCK_SIZE = 512  /* bytes */ };

			size_t const max_adma_request_size = 64*1024 - 4; /* bytes */

			/*
			 * sanity check
			 *
			 * XXX An alternative to this sanity check would be to expose
			 *     the maximum DMA transfer size to the driver and let the
			 *     driver partition large requests into ones that are
			 *     supported by the controller.
			 */
			if (block_count*BLOCK_SIZE > max_adma_request_size*ADMA_DESC_MAX_ENTRIES) {
				PERR("Block request too large");
				return false;
			}

			/*
			 * Each ADMA descriptor can transfer up to MAX_ADMA_REQUEST_SIZE
			 * bytes. If the request is larger, we generate a list of ADMA
			 * descriptors.
			 */

			size_t const total_bytes = block_count*BLOCK_SIZE;

			/* number of bytes for which descriptors have been created */
			addr_t consumed_bytes = 0;

			for (int index = 0; consumed_bytes < total_bytes; index++) {

				size_t const remaining_bytes = total_bytes - consumed_bytes;

				/* clamp current request to maximum ADMA request size */
				size_t const curr_bytes = Genode::min(max_adma_request_size,
				                                      remaining_bytes);
				/*
				 * Assemble new ADMA descriptor
				 */
				Adma_desc::access_t desc = 0;
				Adma_desc::Address::set(desc, out_buffer_phys + consumed_bytes);
				Adma_desc::Length::set(desc, curr_bytes);

				/* set action to transfer */
				Adma_desc::Act1::set(desc, 0);
				Adma_desc::Act2::set(desc, 1);

				Adma_desc::Valid::set(desc, 1);

				/*
				 * Let the last descriptor generate transfer-complete interrupt
				 */
				if (consumed_bytes + curr_bytes == total_bytes){
					Adma_desc::Ent::set(desc, 1);
				}

				/* install descriptor into ADMA descriptor table */
				_adma_desc[index] = desc;

				consumed_bytes += curr_bytes;
			}

			return true;
		}

		bool _wait_for_transfer_complete_irq()
		{
			/*
			 * XXX For now, the driver works fully synchronous. We merely use
			 *     the interrupt mechanism to yield CPU time to concurrently
			 *     running processes.
			 */
			for (;;) {
				_irq.wait_for_irq();

				/* check for transfer completion */
				if (read<Stat::Tc>() == 1) {

					/* clear transfer-completed bit */
					write<Stat::Tc>(1);

					if (read<Stat>() != 0)
						PWRN("unexpected state (Stat: 0x%08x Blen: 0x%x Nblk: %d)",
						     read<Stat>(), read<Blk::Blen>(), read<Blk::Nblk>());

					return true;
				}

				PWRN("unexpected interrupt, Stat: 0x%08x", read<Stat>());
			}
		}

	public:

		enum { IRQ_NUMBER = Genode::Board_base::USDHC4_IRQ };

		/**
		 * Constructor
		 *
		 * \param mmio_base  local base address of MMIO registers
		 */
		Imx6_usdhc_controller(Genode::addr_t const mmio_base, Delayer &delayer,
		                       bool use_dma)
		:
			Usdhc(mmio_base), _delayer(delayer), _card_info(_init()),
			_use_dma(use_dma),
			_adma_desc_ds(Genode::env()->ram_session(),
			              ADMA_DESC_MAX_ENTRIES*sizeof(Adma_desc::access_t),
			              Genode::UNCACHED),
			_adma_desc(_adma_desc_ds.local_addr<Adma_desc::access_t>()),
			_adma_desc_phys(Genode::Dataspace_client(_adma_desc_ds.cap()).phys_addr()),
			_irq(IRQ_NUMBER)
		{ }


		/****************************************
		 ** Sd_card::Host_controller interface **
		 ****************************************/

		bool _issue_command(Sd_card::Command_base const &command)
		{
			if (verbose)
				PLOG("-> index=0x%08x, arg=0x%08x, rsp_type=%d",
				     command.index, command.arg, command.rsp_type);

			if (!wait_for<Pstate::Cmdi>(0, _delayer)) {
				PERR("wait for Pstate::Cmdi timed out");
				return false;
			}

			/* write command argument */
			write<Arg>(command.arg);

			/* assemble command register */
			Cmd::access_t cmd = 0;
			Cmd::Index::set(cmd, command.index);
			Mixctrl::access_t mixt = 0;
			if (command.transfer != Sd_card::TRANSFER_NONE) {

				if (!wait_for<Pstate::Dti>(0, _delayer)) {
					PERR("wait for Pstate::Dti timed out");
					return false;
				}

				Cmd::Dp::set(cmd);
				if (_use_dma)
					Mixctrl::De::set(mixt);

				if (command.index == Sd_card::Read_multiple_block::INDEX 
				 || command.index == Sd_card::Write_multiple_block::INDEX) {
					Mixctrl::Bce::set(mixt);
					Mixctrl::Msbs::set(mixt);
					Mixctrl::Acen::set(mixt);

				}

				/* set data-direction bit depending on the command */
				bool const read = command.transfer == Sd_card::TRANSFER_READ;
				Mixctrl::Ddir::set(mixt, read ? Mixctrl::Ddir::READ : Mixctrl::Ddir::WRITE);
				write<Mixctrl>(mixt);
			}

			Cmd::access_t rsp_type = 0;
			switch (command.rsp_type) {
			case Sd_card::RESPONSE_NONE:             rsp_type = Cmd::Rsp_type::RESPONSE_NONE;             break;
			case Sd_card::RESPONSE_136_BIT:          rsp_type = Cmd::Rsp_type::RESPONSE_136_BIT;          break;
			case Sd_card::RESPONSE_48_BIT:           rsp_type = Cmd::Rsp_type::RESPONSE_48_BIT;           break;
			case Sd_card::RESPONSE_48_BIT_WITH_BUSY: rsp_type = Cmd::Rsp_type::RESPONSE_48_BIT_WITH_BUSY; break;
			}
			Cmd::Rsp_type::set(cmd, rsp_type);

			/* write command */
			write<Cmd>(cmd);

			bool result = false;

			/* wait until command is completed, return false on timeout */
			for (unsigned long i = 0; i < 1000*1000; i++) {
				Stat::access_t const stat = read<Stat>();
				if (Stat::Cc::get(stat) == 1) {
					result = true;
					break;
				}
			}

			if (verbose)
				PLOG("<- %s", result ? "succeeded" : "timed out");

			/* clear status of command-completed bit */
			if(result == true)
				write<Stat::Cc>(1);

			read<Stat>();

			return result;
		}

		Sd_card::Card_info card_info() const
		{
			return _card_info;
		}

		Sd_card::Cid _read_cid()
		{
			Sd_card::Cid cid;
			cid.raw_0 = read<Rsp0>();
			cid.raw_1 = read<Rsp1>();
			cid.raw_2 = read<Rsp2>();
			cid.raw_3 = read<Rsp3>();
			return cid;
		}

		Sd_card::Csd _read_csd()
		{
			/**
			 * USDHC returns CSD register right shifted by 8 bits
			 * Correct it here and return
			 */
			Sd_card::Csd csd;
			csd.csd3 = read<Rsp3>();
			csd.csd3 = csd.csd3<<8;

			csd.csd2 = read<Rsp2>();
			csd.csd3 |= csd.csd2>>24;
			csd.csd2 = csd.csd2<<8;

			csd.csd1 = read<Rsp1>();
			csd.csd2 |= csd.csd1>>24;
			csd.csd1 = csd.csd1<<8;

			csd.csd0 = read<Rsp0>();
			csd.csd1 |= csd.csd0>>24;
			csd.csd0 = csd.csd0<<8;
			return csd;
		}

		unsigned _read_rca()
		{
			return Sd_card::Send_relative_addr::Response::Rca::get(read<Rsp0>());
		}

		/**
		 * Read data blocks from SD card
		 *
		 * \return true on success
		 */
		bool read_blocks(size_t block_number, size_t block_count, char *out_buffer)
		{
			PERR("Polling IO not supported. Please use DMA");
			return false;
		}

		/**
		 * Write data blocks to SD card
		 *
		 * \return true on success
		 */
		bool write_blocks(size_t block_number, size_t block_count, char const *buffer)
		{
			PERR("Polling IO not supported. Please use DMA");
			return false;
		}

		/**
		 * Read data blocks from SD card via master DMA
		 *
		 * \return true on success
		 */
		bool read_blocks_dma(size_t block_number, size_t block_count,
		                     Genode::addr_t out_buffer_phys)
		{
			using namespace Sd_card;

			/* USDHC expects this */
			if (!issue_command(Set_blocklen(0x200))) {
				PERR("Set block length failed, Stat: 0x%08x, Host: %08x", read<Stat>(), read<Hctl>());
				return false;
			}

			write<Blk::Blen>(0x200);
			write<Blk::Nblk>(block_count);

			if(!_setup_adma_descriptor_table(block_count, out_buffer_phys))
				PERR("setup adma failed\n");

			/* select DMA mode and enable DMA interrupts */
			write<Hctl::Dmasel>(Hctl::Dmasel::ADMA2);
			write<Ie::Dmae>(1);
			write<Ie::Dma_enable>(1);
			write<Ise::Dma_sigen>(1);
			write<Ise::Dmae_sigen>(1);

			if (!issue_command(Read_multiple_block(block_number))) {
				PERR("Read_multiple_block failed, Stat: 0x%08x, Host: 0x%08x", read<Stat>(), read<Hctl>());
				return false;
			}

			return _wait_for_transfer_complete_irq();
		}

		/**
		 * Write data blocks to SD card via master DMA
		 *
		 * \return true on success
		 */
		bool write_blocks_dma(size_t block_number, size_t block_count,
		                      Genode::addr_t buffer_phys)
		{
			using namespace Sd_card;

			/* USDHC expects this */
			if (!issue_command(Set_blocklen(0x200))) {
				PERR("Set block length failed, Stat: 0x%08x, Host: %08x", read<Stat>(), read<Hctl>());
				return false;
			}

			write<Blk::Blen>(0x200);
			write<Blk::Nblk>(block_count);

			_setup_adma_descriptor_table(block_count, buffer_phys);

			/* select DMA mode and enable DMA interrupts */
			write<Hctl::Dmasel>(Hctl::Dmasel::ADMA2);
			write<Ie::Dmae>(1);
			write<Ie::Dma_enable>(1);
			write<Ise::Dma_sigen>(1);
			write<Ise::Dmae_sigen>(1);

			if (!issue_command(Write_multiple_block(block_number))) {
				PERR("Write_multiple_block failed");
				return false;
			}

			return _wait_for_transfer_complete_irq();
		}
};

#endif /* _USDHC_H_ */

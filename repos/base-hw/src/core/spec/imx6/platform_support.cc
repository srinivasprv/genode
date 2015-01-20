/*
 * \brief   Specific core implementations
 * \author  Praveen Srinivas (IIT Madras, India)
 * \author  Chirag Garg (IIT Madras, India)
 * \author  Stefan Kalkowski
 * \date    2015-01-20
 */

/*
 * Copyright (C) 2012-2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* core includes */
#include <platform.h>
#include <board.h>
#include <pic.h>
#include <cpu.h>
#include <unmanaged_singleton.h>

using namespace Genode;

Native_region * Platform::_ram_regions(unsigned const i)
{
	static Native_region _regions[] =
	{
		{ Board::RAM0_BASE, Board::RAM0_SIZE },
	};
	return i < sizeof(_regions)/sizeof(_regions[0]) ? &_regions[i] : 0;
}


Native_region * Platform::_mmio_regions(unsigned const i)
{
	static Native_region _regions[] =
	{
		{ Board::MMIO_BASE, Board::MMIO_SIZE },
	};
	return i < sizeof(_regions)/sizeof(_regions[0]) ? &_regions[i] : 0;
}


Native_region * Platform::_core_only_mmio_regions(unsigned const i)
{
	static Native_region _regions[] =
	{
		/* core UART */
		{ Board::UART_2_MMIO_BASE, Board::UART_2_MMIO_SIZE },

		/* core timer */
		{ Board::EPIT_1_MMIO_BASE, Board::EPIT_1_MMIO_SIZE },

		/* interrupt controller */
		{ Board::CORTEX_A9_PRIVATE_MEM_BASE, Board::CORTEX_A9_PRIVATE_MEM_SIZE },
		
		/* l2 cache controller */
		{ Board::PL310_MMIO_BASE, Board::PL310_MMIO_SIZE }
	};
	return i < sizeof(_regions)/sizeof(_regions[0]) ? &_regions[i] : 0;
}


Cpu::User_context::User_context() { cpsr = Psr::init_user(); }

/*
 * \brief  Serial output driver for core
 * \author Praveen Srinivas (IIT Madras, India)
 * \author Chirag Garg (IIT Madras, India)
 * \author Stefan Kalkowski
 * \date   2015-01-20
 */

/*
 * Copyright (C) 2012-2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _SERIAL_H_
#define _SERIAL_H_

/* core includes */
#include <board.h>

/* Genode includes */
#include <drivers/uart/imx_uart_base.h>

namespace Genode
{
	/**
	 * Serial output driver for core
	 */
	class Serial : public Imx_uart_base
	{
		public:

			/**
			 * Constructor
			 *
			 * XXX: The 'baud_rate' argument is ignored for now.
			 */
			Serial(unsigned) : Imx_uart_base(Board::UART_2_MMIO_BASE) { }
	};
}

#endif /* _SERIAL_H_ */

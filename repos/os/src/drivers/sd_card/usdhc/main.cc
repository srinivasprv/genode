/*
 * \brief  SDHC card driver for Freescale-i.MX6 platform
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

/* Genode includes */
#include <base/printf.h>
#include <os/server.h>

/* local includes */
#include <driver.h>


struct Main
{
	Server::Entrypoint &ep;

	struct Factory : Block::Driver_factory
	{
		Block::Driver *create() {
			return new (Genode::env()->heap()) Block::Imx6_driver(true); }

		void destroy(Block::Driver *driver) {
			Genode::destroy(Genode::env()->heap(),
			                static_cast<Block::Imx6_driver *>(driver)); }
	} factory;

	Block::Root           root;

	Main(Server::Entrypoint &ep)
	: ep(ep), 
	  root(ep, Genode::env()->heap(), factory)
	{
		Genode::printf("--- i.MX6 SDHC card driver ---\n");

		Genode::env()->parent()->announce(ep.manage(root));
	}
};


/************
 ** Server **
 ************/

namespace Server {
	char const *name()             { return "sd_card_ep";        }
	size_t stack_size()            { return 2*1024*sizeof(long); }
	void construct(Entrypoint &ep) { static Main server(ep);     }
}

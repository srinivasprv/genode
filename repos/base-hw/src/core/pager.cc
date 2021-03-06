/*
 * \brief  Pager implementations that are specific for the HW-core
 * \author Martin Stein
 * \date   2012-03-29
 */

/*
 * Copyright (C) 2012-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/pager.h>
#include <base/printf.h>

using namespace Genode;


/*************
 ** Mapping **
 *************/

Mapping::Mapping(addr_t const va, addr_t const pa,
                 Cache_attribute const c, bool const io,
                 unsigned const sl2, bool const w)
:
	virt_address(va), phys_address(pa), cacheable(c),
	io_mem(io), size_log2(sl2), writable(w)
{ }


Mapping::Mapping()
:
virt_address(0), phys_address(0), cacheable(CACHED),
	io_mem(0), size_log2(0), writable(0)
{ }


void Mapping::prepare_map_operation() { }


/***************
 ** Ipc_pager **
 ***************/

addr_t Ipc_pager::fault_ip() const { return _fault.ip; }

addr_t Ipc_pager::fault_addr() const { return _fault.addr; }

bool Ipc_pager::is_write_fault() const { return _fault.writes; }

void Ipc_pager::set_reply_mapping(Mapping m) { _mapping = m; }


/******************
 ** Pager_object **
 ******************/

Thread_capability Pager_object::thread_cap() const { return _thread_cap; }

void Pager_object::thread_cap(Thread_capability const & c) { _thread_cap = c; }

Signal * Pager_object::_signal() const { return (Signal *)_signal_buf; }

void Pager_object::wake_up() { fault_resolved(); }

void Pager_object::exception_handler(Signal_context_capability) { }


Pager_object::Pager_object(unsigned const badge, Affinity::Location)
:
	_signal_valid(0),
	_badge(badge)
{ }


unsigned Pager_object::signal_context_id() const
{
	return _signal_context_cap.dst();
}


/***************************
 ** Pager_activation_base **
 ***************************/

void Pager_activation_base::ep(Pager_entrypoint * const ep) { _ep = ep; }


Pager_activation_base::Pager_activation_base(char const * const name,
                                             size_t const stack_size)
:
	Thread_base(0, name, stack_size), _cap_valid(Lock::LOCKED), _ep(0)
{ }


Native_capability Pager_activation_base::cap()
{
	if (!_cap.valid()) { _cap_valid.lock(); }
	return _cap;
}


/**********************
 ** Pager_entrypoint **
 **********************/

void Pager_entrypoint::dissolve(Pager_object * const o)
{
	remove_locked(o);
	o->stop_paging();
	_activation->Signal_receiver::dissolve(o);
}


Pager_entrypoint::Pager_entrypoint(Cap_session *,
                                   Pager_activation_base * const activation)
:
	_activation(activation)
{
	_activation->ep(this);
}


Pager_capability Pager_entrypoint::manage(Pager_object * const o)
{
	unsigned const d = _activation->cap().dst();
	unsigned const b = o->badge();
	auto const p = reinterpret_cap_cast<Pager_object>(Native_capability(d, b));
	o->start_paging(_activation->Signal_receiver::manage(o), p);
	insert(o);
	return p;
}

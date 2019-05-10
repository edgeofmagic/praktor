/*
 * The MIT License
 *
 * Copyright 2017 David Curtis.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef ASYNC_LOOP_UV_H
#define ASYNC_LOOP_UV_H

#include "uv_error.h"
#include <deque>
#include <async/loop.h>
#include <mutex>
#include <uv.h>

using async::ip::endpoint;
using util::mutable_buffer;
using async::transceiver;
using async::acceptor;
using async::channel;
using async::options;
using async::timer;
using async::loop;

class resolve_req_uv
{
public:
	resolve_req_uv(std::string const& hostname, loop::resolve_handler handler)
		: m_hostname{hostname}, m_handler{std::move(handler)}
	{
		assert(reinterpret_cast<uv_getaddrinfo_t*>(this) == &m_uv_req);
	}

	void
	start(uv_loop_t* lp, std::error_code& err);

private:
	uv_getaddrinfo_t      m_uv_req;
	std::string           m_hostname;
	loop::resolve_handler m_handler;

	static void
	on_resolve(uv_getaddrinfo_t* req, int status, struct addrinfo* result);
};

class loop_uv;
class udp_transceiver_uv;

struct loop_data
{
	std::weak_ptr<loop_uv> m_impl_wptr;
	std::shared_ptr<loop_uv>
	get_loop_ptr();
};

class loop_uv : public loop
{
public:
	friend class loop;

	using ptr  = std::shared_ptr<loop_uv>;
	using wptr = std::weak_ptr<loop_uv>;

	using void_handler = std::function<void()>;

	struct use_default_loop
	{};

	loop_uv(use_default_loop flag);    // only use to construct for default loop

	loop_uv();

	virtual ~loop_uv();

	static ptr
	create_from_default();

	void
	init(wptr self);

	virtual bool
	is_alive() const override;

private:
	loop_uv(loop_uv const&) = delete;
	loop_uv(loop_uv&&)      = delete;

	static ptr
	get_loop_ptr(uv_loop_t* lp)
	{
		return reinterpret_cast<loop_data*>(lp->data)->get_loop_ptr();
	}

	loop_uv&
	operator=(loop_uv const&)
			= delete;

	loop_uv&
	operator=(loop_uv&&)
			= delete;

	virtual timer::ptr
	really_create_timer(std::error_code& err) override;

	virtual timer::ptr
	really_create_timer(std::error_code& err, timer::handler&& handler) override;

	virtual timer::ptr
	really_create_timer_void(std::error_code& err, timer::void_handler&& handler) override;

	virtual int
	really_run(std::error_code& err) override;

	virtual int
	really_run_once(std::error_code& err) override;

	virtual int
	really_run_nowait(std::error_code& err) override;

	virtual void
	really_stop(std::error_code& err) override;

	static void
	on_walk(uv_handle_t* handle, void*);

	virtual void
	really_close(std::error_code& err) override;    // probably should NOT be called from any handler

	virtual acceptor::ptr
	really_create_acceptor(std::error_code& err) override;
	
	virtual acceptor::ptr
	really_create_acceptor(options const& opt, std::error_code& err, acceptor::connection_handler&& handler) override;

	virtual channel::ptr
	really_connect_channel(options const& opt, std::error_code& err, channel::connect_handler&& handler) override;

	virtual transceiver::ptr
	really_create_transceiver(options const& opt, std::error_code& err, transceiver::receive_handler&& handler)
			override;

	virtual transceiver::ptr
	really_create_transceiver(options const& opt, std::error_code& err) override;

	util::shared_ptr<udp_transceiver_uv>
	setup_transceiver(options const& opts, std::error_code& err);

	virtual void
	really_resolve(std::string const& hostname, std::error_code& err, resolve_handler&& handler) override;

	virtual void
	really_dispatch(std::error_code& err, loop::dispatch_handler&& handler) override;

	virtual void
	really_dispatch_void(std::error_code& err, loop::dispatch_void_handler&& handler) override;

	virtual void
	really_schedule(std::chrono::milliseconds timeout, std::error_code& err, loop::scheduled_handler&& handler) override;

	virtual void
	really_schedule_void(std::chrono::milliseconds timeout, std::error_code& err, loop::scheduled_void_handler&& handler) override;

	bool
	try_dispatch_front();

	static void
	on_async(uv_async_t* handle);

	void
	drain_dispatch_queue()
	{
		while (try_dispatch_front())
			;
	}

	uv_async_t                         m_async_handle;
	std::deque<void_handler>           m_dispatch_queue;
	std::recursive_mutex               m_dispatch_queue_mutex;
	uv_loop_t*                         m_uv_loop;
	loop_data                          m_data;
	bool                               m_is_default_loop;
};

#endif    // ASYNC_LOOP_UV_H
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

#include "loop_uv.h"
#include "tcp_uv.h"
#include "timer_uv.h"
#include "udp_uv.h"

using async::ip::address;

void
resolve_req_uv::start(uv_loop_t* lp, std::error_code& err)
{
	err.clear();
	auto status = uv_getaddrinfo(lp, &m_uv_req, on_resolve, m_hostname.c_str(), nullptr, nullptr);
	UV_ERROR_CHECK(status, err, exit);
exit:
	return;
}

void
resolve_req_uv::on_resolve(uv_getaddrinfo_t* req, int status, struct addrinfo* result)
{
	auto                request = reinterpret_cast<resolve_req_uv*>(req);
	std::error_code     err     = map_uv_error(status);
	std::deque<address> addresses;

	if (!err)
	{
		for (auto info = result; info != nullptr; info = info->ai_next)
		{
			struct sockaddr_storage storage;

			memset(&storage, 0, sizeof(storage));
			memcpy(&storage, info->ai_addr, info->ai_addrlen);

			address addr;

			if (info->ai_family == AF_INET)
			{
				addr = reinterpret_cast<struct sockaddr_in*>(info->ai_addr)->sin_addr;
			}
			else if (info->ai_family == AF_INET6)
			{
				addr = reinterpret_cast<struct sockaddr_in6*>(info->ai_addr)->sin6_addr;
			}

			auto it = std::find(addresses.begin(), addresses.end(), addr);

			if (it == addresses.end())
			{
				addresses.emplace_back(addr);
			}
		}
	}
	uv_freeaddrinfo(result);
	request->m_handler(request->m_hostname, std::move(addresses), err);
	request->m_handler = nullptr;
	delete request;
}


loop_uv::loop_uv(use_default_loop flag) : m_uv_loop{uv_default_loop()}, m_is_default_loop{true} {}

loop_uv::loop_uv() : m_uv_loop{new uv_loop_t}, m_is_default_loop{false}
{
	uv_loop_init(m_uv_loop);
}

void
loop_uv::init(loop_uv::wptr self)
{
	m_data.m_impl_wptr = self;
	uv_loop_set_data(m_uv_loop, &m_data);
	uv_async_init(m_uv_loop, &m_async_handle, on_async);
}

loop_uv::~loop_uv()
{
	std::error_code err;
	close(err);
}

timer::ptr
loop_uv::really_create_timer(std::error_code& err)
{
	err.clear();
	timer_uv::ptr result;

	if (!m_uv_loop)
	{
		err = make_error_code(async::errc::loop_closed);
		goto exit;
	}

	result = util::make_shared<timer_uv>(m_uv_loop, err);
	result->init(result);
	if (err)
		goto exit;

exit:
	return result;
}

timer::ptr
loop_uv::really_create_timer(std::error_code& err, timer::handler&& handler)
{
	err.clear();
	timer_uv::ptr result;

	if (!handler)
	{
		err = make_error_code(std::errc::invalid_argument);
		goto exit;
	}

	if (!m_uv_loop)
	{
		err = make_error_code(async::errc::loop_closed);
		goto exit;
	}

	result = util::make_shared<timer_uv>(m_uv_loop, err, std::move(handler));
	result->init(result);
	if (err)
		goto exit;

exit:
	return result;
}

timer::ptr
loop_uv::really_create_timer_void(std::error_code& err, timer::void_handler&& handler)
{
	err.clear();
	timer_uv::ptr result;

	if (!handler)
	{
		err = make_error_code(std::errc::invalid_argument);
		goto exit;
	}

	if (!m_uv_loop)
	{
		err = make_error_code(async::errc::loop_closed);
		goto exit;
	}
	result = util::make_shared<timer_uv>(
			m_uv_loop, err, [=, handler{std::move(handler)}](async::timer::ptr) { handler(); });
	result->init(result);
	if (err)
		goto exit;

exit:
	return result;
}

int
loop_uv::really_run(std::error_code& err)
{
	err.clear();
	int result = 0;
	if (!m_uv_loop)
	{
		err = make_error_code(async::errc::loop_closed);
		goto exit;
	}

	{
		result = uv_run(m_uv_loop, UV_RUN_DEFAULT);
		// UV_ERROR_CHECK(status, err, exit);
	}

exit:
	return result;
}

int
loop_uv::really_run_once(std::error_code& err)
{
	err.clear();
	int result = 0;
	if (!m_uv_loop)
	{
		err = make_error_code(async::errc::loop_closed);
		goto exit;
	}

	{
		result = uv_run(m_uv_loop, UV_RUN_ONCE);
		// UV_ERROR_CHECK(status, err, exit);
	}

exit:
	return result;
}

int
loop_uv::really_run_nowait(std::error_code& err)
{
	err.clear();
	int result = 0;
	if (!m_uv_loop)
	{
		err = make_error_code(async::errc::loop_closed);
		goto exit;
	}

	{
		result = uv_run(m_uv_loop, UV_RUN_NOWAIT);
		// UV_ERROR_CHECK(status, err, exit);
	}

exit:
	return result;
}

void
loop_uv::really_stop(std::error_code& err)
{
	err.clear();
	if (!m_uv_loop)
	{
		err = make_error_code(async::errc::loop_closed);
	}
	uv_stop(m_uv_loop);
exit:
	return;
}

void
loop_uv::on_walk(uv_handle_t* handle, void*)
{
	if (handle)
	{
		auto handle_type = uv_handle_get_type(handle);
		switch (handle_type)
		{
			case uv_handle_type::UV_TIMER:
				if (!uv_is_closing(handle))
				{
					uv_close(handle, timer_uv::on_timer_close);
				}
				break;
			case uv_handle_type::UV_TCP:
				if (!uv_is_closing(handle))
				{
					uv_close(handle, tcp_base_uv::on_close);
				}
				break;
			case uv_handle_type::UV_ASYNC:
				if (!uv_is_closing(handle))
				{
					uv_close(handle, nullptr);
				}
				break;
			case uv_handle_type::UV_UDP:
				if (!uv_is_closing(handle))
				{
					uv_close(handle, udp_transceiver_uv::on_close);
				}
				break;
			default:
				std::cout << "closing loop handles, unexpected handle type: " << handle_type << std::endl;
				break;
		}
	}
}

void
loop_uv::really_close(std::error_code& err)    // probably should NOT be called from any handler
{
	err.clear();
	int status = 0;

	if (!m_uv_loop)
	{
		err = make_error_code(async::errc::loop_closed);
		goto exit;
	}

	status = uv_loop_close(m_uv_loop);
	if (status == UV_EBUSY)
	{
		uv_walk(m_uv_loop, on_walk, nullptr);
		status = uv_run(m_uv_loop, UV_RUN_DEFAULT);    // is this cool?
		UV_ERROR_CHECK(status, err, exit);
		status = uv_loop_close(m_uv_loop);
		UV_ERROR_CHECK(status, err, exit);
	}
	else if (status < 0)
	{
		UV_ERROR_CHECK(status, err, exit);
	}
	if (!m_is_default_loop)
	{
		delete m_uv_loop;
	}
	m_uv_loop = nullptr;
exit:
	return;
}

acceptor::ptr
loop_uv::really_create_acceptor(std::error_code& err)
{
	err.clear();
	util::shared_ptr<tcp_acceptor_uv> acceptor;

	if (!m_uv_loop)
	{
		err = make_error_code(async::errc::loop_closed);
		goto exit;
	}

	acceptor = util::make_shared<tcp_acceptor_uv>();
	acceptor->init(m_uv_loop, acceptor, err);
exit:
	return acceptor;
}

// acceptor::ptr
// loop_uv::really_create_acceptor(options const& opt, std::error_code& err, acceptor::connection_handler handler)
// {
// 	err.clear();
// 	util::shared_ptr<tcp_acceptor_uv> acceptor;

// 	if (!handler)
// 	{
// 		err = make_error_code(std::errc::invalid_argument);
// 		goto exit;
// 	}

// 	if (!m_uv_loop)
// 	{
// 		err = make_error_code(async::errc::loop_closed);
// 		goto exit;
// 	}

// 	acceptor = util::make_shared<tcp_acceptor_uv>(opt.endpoint(), std::move(handler));
// 	acceptor->init(m_uv_loop, acceptor, opt, err);
// exit:
// 	return acceptor;
// }

acceptor::ptr
loop_uv::really_create_acceptor(options const& opt, std::error_code& err, acceptor::connection_handler&& handler)
{
	err.clear();
	util::shared_ptr<tcp_acceptor_uv> acceptor;

	if (!handler)
	{
		err = make_error_code(std::errc::invalid_argument);
		goto exit;
	}

	if (!m_uv_loop)
	{
		err = make_error_code(async::errc::loop_closed);
		goto exit;
	}

	acceptor = util::make_shared<tcp_acceptor_uv>();
	acceptor->init(m_uv_loop, acceptor, err);
	if (err) goto exit;
	acceptor->bind(opt, err);
	if (err) goto exit;
	acceptor->listen(err, std::move(handler));
exit:
	return acceptor;
}

channel::ptr
loop_uv::really_connect_channel(options const& opt, std::error_code& err, channel::connect_handler&& handler)
{
	err.clear();
	util::shared_ptr<tcp_channel_uv> cp;

	if (!handler)
	{
		err = make_error_code(std::errc::invalid_argument);
		goto exit;
	}

	if (!m_uv_loop)
	{
		err = make_error_code(async::errc::loop_closed);
		goto exit;
	}

	if (opt.framing())
	{
		cp = util::make_shared<tcp_framed_channel_uv>();
	}
	else
	{
		cp = util::make_shared<tcp_channel_uv>();
	}
	cp->init(m_uv_loop, cp, err);
	if (err)
		goto exit;
	cp->connect(opt.endpoint(), err, std::move(handler));
exit:
	return cp;
}

udp_transceiver_uv::ptr
loop_uv::setup_transceiver(options const& opts, std::error_code& err)
{
	err.clear();
	udp_transceiver_uv::ptr tp;

	if (!m_uv_loop)
	{
		err = make_error_code(async::errc::loop_closed);
		goto exit;
	}

	tp = util::make_shared<udp_transceiver_uv>();

	tp->init(m_uv_loop, tp, err);
	if (err)
		goto exit;

	tp->bind(opts, err);
	if (err)
		goto exit;

exit:
	return tp;
}

transceiver::ptr
loop_uv::really_create_transceiver(options const& opts, std::error_code& err, transceiver::receive_handler&& handler)
{
	err.clear();
	udp_transceiver_uv::ptr tp;

	if (!handler)
	{
		err = make_error_code(std::errc::invalid_argument);
		goto exit;
	}

	tp = setup_transceiver(opts, err);
	if (err)
		goto exit;

	tp->start_receive(err, std::move(handler));

exit:
	return tp;
}

transceiver::ptr
loop_uv::really_create_transceiver(options const& opts, std::error_code& err)
{
	err.clear();
	util::shared_ptr<udp_transceiver_uv> tp;

	tp = setup_transceiver(opts, err);
	if (err)
		goto exit;

exit:
	return tp;
}


void
loop_uv::really_resolve(std::string const& hostname, std::error_code& err, resolve_handler&& handler)
{
	err.clear();

	if (!handler)
	{
		err = make_error_code(std::errc::invalid_argument);
		goto exit;
	}

	if (!m_uv_loop)
	{
		err = make_error_code(async::errc::loop_closed);
		goto exit;
	}

	{
		resolve_req_uv* req = new resolve_req_uv{hostname, std::move(handler)};
		req->start(m_uv_loop, err);
	}

exit:
	return;
}

loop_uv::ptr
loop_data::get_loop_ptr()
{
	return m_impl_wptr.lock();
}

loop_uv::ptr
loop_uv::create_from_default()
{
	std::shared_ptr<loop_uv> default_loop = std::make_shared<loop_uv>(use_default_loop{});
	default_loop->init(default_loop);
	return default_loop;
}

loop::ptr
loop::get_default()
{
	static std::shared_ptr<loop_uv> default_loop = loop_uv::create_from_default();
	return default_loop;
}

loop::ptr
loop::create()
{
	auto lp = std::make_shared<loop_uv>();
	lp->init(lp);
	return lp;
}

void
loop_uv::really_dispatch(std::error_code& err, loop::dispatch_handler&& handler)
{
	err.clear();
	if (!handler)
	{
		err = make_error_code(std::errc::invalid_argument);
		goto exit;
	}

	if (!m_uv_loop)
	{
		err = make_error_code(async::errc::loop_closed);
		goto exit;
	}

	{
		std::lock_guard<std::recursive_mutex> guard(m_dispatch_queue_mutex);
		m_dispatch_queue.emplace_back([=, handler{std::move(handler)}]() { handler(m_data.get_loop_ptr()); });
	}

	{
		auto stat = uv_async_send(&m_async_handle);
		if (stat < 0)
		{
			err = map_uv_error(stat);
		}
	}
exit:
	return;
}

void
loop_uv::really_dispatch_void(std::error_code& err, loop::dispatch_void_handler&& handler)
{
	err.clear();
	if (!handler)
	{
		err = make_error_code(std::errc::invalid_argument);
		goto exit;
	}

	if (!m_uv_loop)
	{
		err = make_error_code(async::errc::loop_closed);
		goto exit;
	}

	{
		std::lock_guard<std::recursive_mutex> guard(m_dispatch_queue_mutex);
		m_dispatch_queue.emplace_back(std::move(handler));
	}

	{
		auto stat = uv_async_send(&m_async_handle);
		if (stat < 0)
		{
			err = map_uv_error(stat);
		}
	}
exit:
	return;
}

bool
loop_uv::is_alive() const
{
	return m_uv_loop;
}

void
loop_uv::really_schedule(
		std::chrono::milliseconds                 timeout,
		std::error_code&                          err,
		async::loop::scheduled_handler&& handler)
{
	auto tp = really_create_timer(
			err, [=, handler{std::move(handler)}](async::timer::ptr) { handler(m_data.get_loop_ptr()); });
	if (err)
		goto exit;
	tp->start(timeout, err);
exit:
	return;
}

void
loop_uv::really_schedule_void(
		std::chrono::milliseconds                      timeout,
		std::error_code&                               err,
		async::loop::scheduled_void_handler&& handler)
{
	auto tp = really_create_timer(err, [handler{std::move(handler)}](async::timer::ptr) { handler(); });
	if (err)
		goto exit;
	tp->start(timeout, err);
exit:
	return;
}

bool
loop_uv::try_dispatch_front()
{
	bool ok = false;

	void_handler handler;

	{
		std::lock_guard<std::recursive_mutex> guard(m_dispatch_queue_mutex);

		if (!m_dispatch_queue.empty())
		{
			handler = std::move(m_dispatch_queue.front());
			m_dispatch_queue.pop_front();
			ok = true;
		}
	}

	if (ok)
	{
		handler();
	}

	return ok;
}

void
loop_uv::on_async(uv_async_t* handle)
{
	auto lp = get_loop_ptr(reinterpret_cast<uv_handle_t*>(handle)->loop);
	if (lp)
	{
		assert(&lp->m_async_handle == handle);
		lp->drain_dispatch_queue();
	}
}

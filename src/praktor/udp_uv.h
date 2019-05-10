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

#ifndef PRAKTOR_UDP_UV_H
#define PRAKTOR_UDP_UV_H

#include "uv_error.h"
#include <boost/endian/conversion.hpp>
#include <praktor/endpoint.h>
#include <praktor/options.h>
#include <praktor/transceiver.h>
#include <uv.h>

using praktor::ip::endpoint;
using util::mutable_buffer;
using praktor::transceiver;
using praktor::options;

class udp_transceiver_uv;

class udp_send_buf_req_uv
{
public:

	udp_send_buf_req_uv(mutable_buffer&& buf, endpoint const& ep, transceiver::send_buffer_handler handler)
		: m_send_handler{std::move(handler)},
		  m_buffer{std::move(buf)},
		  m_uv_buffer{reinterpret_cast<char*>(m_buffer.data()), m_buffer.size()},
		  m_endpoint{ep}
	{
		assert(reinterpret_cast<uv_udp_send_t*>(this) == &m_uv_send_request);
	}

	int
	start(uv_udp_t* trans)
	{
		return uv_udp_send(&m_uv_send_request, trans, &m_uv_buffer, 1, m_endpoint.get_sockaddr_ptr(), on_send);
	}

private:
	~udp_send_buf_req_uv() {}

	static util::shared_ptr<udp_transceiver_uv>
	get_transceiver_shared_ptr(uv_udp_send_t* req);

	static void
	on_send(uv_udp_send_t* req, int status);

	uv_udp_send_t                    m_uv_send_request;
	mutable_buffer                   m_buffer;
	uv_buf_t                         m_uv_buffer;
	endpoint                         m_endpoint;
	transceiver::send_buffer_handler m_send_handler;
};


class udp_send_bufs_req_uv
{
public:
	udp_send_bufs_req_uv(std::deque<mutable_buffer>&& bufs, endpoint const& ep, praktor::transceiver::send_buffers_handler handler)
		: m_send_handler{std::move(handler)},
		  m_buffers{std::move(bufs)},
		  m_uv_buffers{new uv_buf_t[m_buffers.size()]}
	{
		assert(reinterpret_cast<uv_udp_send_t*>(this) == &m_uv_send_request);

		std::size_t i = 0;
		for (auto it = m_buffers.begin(); it != m_buffers.end(); ++it)
		{
			m_uv_buffers[i].base = reinterpret_cast<char*>(it->data());
			m_uv_buffers[i].len  = it->size();
			++i;
		}
	}

	int
	start(uv_udp_t* trans)
	{
		return uv_udp_send(
				&m_uv_send_request, trans, m_uv_buffers, m_buffers.size(), m_endpoint.get_sockaddr_ptr(), on_send);
	}

private:
	~udp_send_bufs_req_uv()
	{
		if (m_uv_buffers)
		{
			delete[] m_uv_buffers;
		}
	}

	static util::shared_ptr<udp_transceiver_uv>
	get_transceiver_shared_ptr(uv_udp_send_t* req);

	static void
	on_send(uv_udp_send_t* req, int status);

	uv_udp_send_t                     m_uv_send_request;
	std::deque<mutable_buffer>        m_buffers;
	uv_buf_t*                         m_uv_buffers;
	endpoint                          m_endpoint;
	transceiver::send_buffers_handler m_send_handler;
};

class udp_transceiver_uv : public transceiver
{
public:
	using ptr = util::shared_ptr<udp_transceiver_uv>;

	void
	init(uv_loop_t* lp, ptr const& self, std::error_code& err);

	static ptr
	get_shared_ptr(uv_handle_t* handle)
	{
		return get_handle_data(handle)->m_self_ptr;
	}

	static ptr
	get_shared_ptr(uv_udp_t* handle)
	{
		return get_shared_ptr(reinterpret_cast<uv_handle_t*>(handle));
	}

	static udp_transceiver_uv*
	get_raw_ptr(uv_handle_t* handle)
	{
		return get_handle_data(handle)->m_self_ptr.get();
	}

	static udp_transceiver_uv*
	get_raw_ptr(uv_udp_t* handle)
	{
		return get_handle_data(reinterpret_cast<uv_handle_t*>(handle))->m_self_ptr.get();
	}

	void
	bind(options const& opts, std::error_code& err)
	{
		err.clear();
		auto stat = uv_udp_bind(&m_udp_handle, opts.endpoint().get_sockaddr_ptr(), 0);
		if (stat < 0)
		{
			err = map_uv_error(stat);
		}
	}

	virtual ~udp_transceiver_uv() {}

	static void
	on_close(uv_handle_t* handle)
	{
		assert(uv_handle_get_type(handle) == uv_handle_type::UV_UDP);
		auto trans = get_raw_ptr(handle);
		assert(trans->get_handle() == handle);
		trans->clear();
	}

protected:
	void
	clear_handler()
	{
		if (m_close_handler)
		{
			m_close_handler(util::dynamic_pointer_cast<udp_transceiver_uv>(m_data.m_self_ptr));
			m_close_handler = nullptr;
		}
		m_receive_handler = nullptr;
	}

	static void
	on_receive(uv_udp_t* udp_handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags);

	static void
	on_allocate(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

	virtual void
	really_start_receive(std::error_code& err, transceiver::receive_handler&& handler) override;

	virtual void
	stop_receive() override;

	virtual std::shared_ptr<praktor::loop>
	loop() override;

	virtual bool
	is_closing() override;

	virtual void
	really_send(mutable_buffer&& buf, endpoint const& dest, std::error_code& err, send_buffer_handler&& handler)
			override;

	virtual void
	really_send(
			std::deque<mutable_buffer>&& bufs,
			endpoint const&              dest,
			std::error_code&             err,
			send_buffers_handler&&       handler) override;

	virtual bool
	really_close(close_handler&& handler) override;

	struct handle_data
	{
		ptr m_self_ptr;
	};

	udp_transceiver_uv*
	get_self_raw_ptr() const
	{
		return m_data.m_self_ptr.get();
	}

	void
	set_self_ptr(ptr const& self)
	{
		m_data.m_self_ptr = self;
	}

	handle_data*
	get_handle_data()
	{
		return &m_data;
	}

	static handle_data*
	get_handle_data(uv_handle_t* handle)
	{
		return reinterpret_cast<handle_data*>(handle->data);
	}

	static uv_handle_t*
	get_handle_indirect(uv_handle_t* handle)
	{
		return reinterpret_cast<uv_handle_t*>(&(get_handle_data(handle)->m_self_ptr->m_udp_handle));
	}

	uv_udp_t*
	get_udp_handle()
	{
		return &m_udp_handle;
	}

	uv_handle_t*
	get_handle()
	{
		return reinterpret_cast<uv_handle_t*>(&m_udp_handle);
	}

	void
	clear()
	{
		clear_handler();
		m_data.m_self_ptr.reset();
	}

	transceiver::receive_handler m_receive_handler;
	transceiver::close_handler   m_close_handler;
	handle_data                  m_data;
	uv_udp_t                     m_udp_handle;
};

#endif    // PRAKTOR_UDP_UV_H
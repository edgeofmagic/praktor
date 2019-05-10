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

#ifndef PRAKTOR_TCP_UV_H
#define PRAKTOR_TCP_UV_H

#include "uv_error.h"
#include <boost/endian/conversion.hpp>
#include <praktor/endpoint.h>
#include <praktor/options.h>
#include <praktor/tcp.h>
#include <uv.h>

class tcp_channel_uv;
class tcp_acceptor_uv;

using praktor::ip::endpoint;
using util::mutable_buffer;

/** \brief Wraps a libuv connect request object (uv_connect_t).
 * 
 * This class wraps a libuv connect request object (uv_connect_t),
 * holding a callable object of type praktor::channel::connect_handler
 * that will be invoked when the connect request completes.
 * 
 */
class connect_request_uv
{
public:
	/** \brief Constructor
	 * 
	 * Constructs an instance of connect_request_uv.
	 * 
	 * \param handler a callable object that is convertible to praktor::channel::connect_handler, 
	 * which will be invoked when the request completes.
	 */
	connect_request_uv(praktor::channel::connect_handler handler) : m_handler{std::move(handler)}
	{
		// make sure the compiler is doing the expected thing
		assert(reinterpret_cast<uv_connect_t*>(this) == &m_uv_connect_request);
	}

	/** \brief Accessor for underlying uv_connect_t
	 * 
	 * \return a pointer to the underlying uv_connect_t object.
	 */
	uv_connect_t*
	get_uv_connect_request()
	{
		return &m_uv_connect_request;
	}

	/** \brief Clears the completion handler
	 */
	void
	clear_handler()
	{
		m_handler = nullptr;
	}

	/** \brief Callback function for libuv interface (type uv_connect_cb)
	 */
	static void
	on_connect(uv_connect_t* req, int status);

private:
	uv_connect_t                               m_uv_connect_request;
	praktor::channel::connect_handler m_handler;

	static util::shared_ptr<tcp_channel_uv>
	get_channel_shared_ptr(uv_connect_t* req);
};

class tcp_write_buf_req_uv
{
public:
	tcp_write_buf_req_uv(mutable_buffer&& buf, praktor::channel::write_buffer_handler handler)
		: m_write_handler{std::move(handler)},
		  m_buffer{std::move(buf)},
		  m_uv_buffer{reinterpret_cast<char*>(m_buffer.data()), m_buffer.size()}
	{
		assert(reinterpret_cast<uv_write_t*>(this) == &m_uv_write_request);
	}

	int
	start(uv_stream_t* chan)
	{
		return uv_write(&m_uv_write_request, chan, &m_uv_buffer, 1, on_write);
	}

	~tcp_write_buf_req_uv() {}

private:

	static util::shared_ptr<tcp_channel_uv>
	get_channel_shared_ptr(uv_write_t* req);

	static void
	on_write(uv_write_t* req, int status);

	uv_write_t                                      m_uv_write_request;
	mutable_buffer                                  m_buffer;
	uv_buf_t                                        m_uv_buffer;
	praktor::channel::write_buffer_handler m_write_handler;
};


class tcp_write_bufs_req_uv
{
public:
	tcp_write_bufs_req_uv(std::deque<mutable_buffer>&& bufs, praktor::channel::write_buffers_handler handler)
		: m_write_handler{std::move(handler)},
		  m_buffers{std::move(bufs)},
		  m_uv_buffers{new uv_buf_t[m_buffers.size()]}
	{
		assert(reinterpret_cast<uv_write_t*>(this) == &m_uv_write_request);

		std::size_t i = 0;
		for (auto it = m_buffers.begin(); it != m_buffers.end(); ++it)
		{
			m_uv_buffers[i].base = reinterpret_cast<char*>(it->data());
			m_uv_buffers[i].len  = it->size();
			++i;
		}
	}

	int
	start(uv_stream_t* chan)
	{
		return uv_write(&m_uv_write_request, chan, m_uv_buffers, m_buffers.size(), on_write);
	}

	~tcp_write_bufs_req_uv()
	{
		if (m_uv_buffers)
		{
			delete[] m_uv_buffers;
		}
	}

private:

	static util::shared_ptr<tcp_channel_uv>
	get_channel_shared_ptr(uv_write_t* req);

	static void
	on_write(uv_write_t* req, int status);

	uv_write_t                                       m_uv_write_request;
	std::deque<mutable_buffer>                       m_buffers;
	uv_buf_t*                                        m_uv_buffers;
	praktor::channel::write_buffers_handler m_write_handler;
};

class tcp_base_uv
{
public:
	static util::shared_ptr<tcp_base_uv>
	get_base_shared_ptr(uv_stream_t* stream_handle)
	{
		return get_base_shared_ptr(reinterpret_cast<uv_handle_t*>(stream_handle));
	}

	static void
	on_close(uv_handle_t* handle)
	{
		assert(uv_handle_get_type(handle) == uv_handle_type::UV_TCP);
		auto tcp_base = get_base_raw_ptr(handle);
		assert(tcp_base->get_handle() == handle);
		tcp_base->clear();
	}

	const uv_stream_t*
	get_stream_handle() const
	{
		return reinterpret_cast<const uv_stream_t*>(&m_tcp_handle);
	}

	uv_stream_t*
	get_stream_handle()
	{
		return reinterpret_cast<uv_stream_t*>(&m_tcp_handle);
	}

	endpoint
	really_get_endpoint(std::error_code& err);

protected:
	using ptr = util::shared_ptr<tcp_base_uv>;

	struct handle_data
	{
		ptr m_self_ptr;
	};

	virtual ~tcp_base_uv() {}

	tcp_base_uv*
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

	static ptr
	get_base_shared_ptr(uv_handle_t* handle)
	{
		return get_handle_data(handle)->m_self_ptr;
	}

	static uv_handle_t*
	get_handle_indirect(uv_handle_t* handle)
	{
		return reinterpret_cast<uv_handle_t*>(&(get_handle_data(handle)->m_self_ptr->m_tcp_handle));
	}

	static tcp_base_uv*
	get_base_raw_ptr(uv_handle_t* handle)
	{
		return get_handle_data(handle)->m_self_ptr.get();
	}

	uv_tcp_t*
	get_tcp_handle()
	{
		return &m_tcp_handle;
	}

	uv_handle_t*
	get_handle()
	{
		return reinterpret_cast<uv_handle_t*>(&m_tcp_handle);
	}

	std::shared_ptr<praktor::loop>
	get_loop();

	virtual void
	clear_handler()
			= 0;

	void
	clear()
	{
		clear_handler();
		m_data.m_self_ptr.reset();
	}

	handle_data m_data;
	uv_tcp_t    m_tcp_handle;
};

class tcp_channel_uv : public tcp_base_uv, public praktor::tcp_channel
{
public:
	using ptr = util::shared_ptr<tcp_channel_uv>;

	void
	init(uv_loop_t* lp, ptr const& self, std::error_code& err);

	void
	connect(praktor::ip::endpoint const& ep, std::error_code& err, praktor::channel::connect_handler handler)
	{
		err.clear();
		sockaddr_storage saddr;
		ep.to_sockaddr(saddr);
		auto req  = new connect_request_uv(std::move(handler));
		int  stat = uv_tcp_connect(
                req->get_uv_connect_request(),
                get_tcp_handle(),
                reinterpret_cast<sockaddr*>(&saddr),
                connect_request_uv::on_connect);
		if (stat < 0)
		{
			err = map_uv_error(stat);
			if (!uv_is_active(get_handle()))
			{
				req->clear_handler();
				delete req;
			}
		}
	}

	virtual endpoint
	get_endpoint(std::error_code& err) override
	{
		return really_get_endpoint(err);
	}

	virtual endpoint
	get_endpoint() override
	{
		std::error_code err;
		return really_get_endpoint(err);
		if (err)
		{
			throw std::system_error{err};
		}
	}

	virtual endpoint
	get_peer_endpoint(std::error_code& err) override;

	virtual endpoint
	get_peer_endpoint() override;

	virtual std::size_t
	get_queue_size() const override
	{
		return get_stream_handle()->write_queue_size;
	}

	virtual void
	set_close_handler(praktor::channel::close_handler&& handler) override
	{
		m_close_handler = std::move(handler);
	}

protected:
	virtual void
	clear_handler() override;

	static void
	on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

	static void
	on_allocate(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

	static void
	on_close(uv_handle_t* handle);

	virtual void
	really_start_read(std::error_code& err, praktor::channel::read_handler&& handler) override;

	virtual void
	stop_read() override;

	virtual std::shared_ptr<praktor::loop>
	loop() override
	{
		return get_loop();
	}

	virtual bool
	is_closing() override;

	virtual void
	really_write(mutable_buffer&& buf, std::error_code& err, praktor::channel::write_buffer_handler&& handler)
			override;


	virtual void
	really_write(
			std::deque<mutable_buffer>&&                       bufs,
			std::error_code&                                   err,
			praktor::channel::write_buffers_handler&& handler) override;

	virtual bool
	really_close(praktor::channel::close_handler&& handler) override;

	virtual bool
	really_close() override;

	praktor::channel::read_handler  m_read_handler;
	praktor::channel::close_handler m_close_handler;
};

class tcp_framed_channel_uv : public tcp_channel_uv
{
public:
	tcp_framed_channel_uv() : m_header_byte_count{0}, m_frame_size{-1} {}
	using ptr = util::shared_ptr<tcp_framed_channel_uv>;

private:
	using frame_size_type               = std::int64_t;
	static constexpr bool reverse_order = boost::endian::order::native != boost::endian::order::big;

	static mutable_buffer
	pack_frame_header(std::uint64_t frame_size)
	{
		const std::uint64_t packed_frame_size = boost::endian::native_to_big(frame_size);
		return mutable_buffer{&packed_frame_size, sizeof(frame_size)};
	}

	static std::uint64_t
	unpack_frame_header(const void* buf_ptr)
	{
		std::uint64_t packed_frame_size{0};
		::memcpy(&packed_frame_size, buf_ptr, sizeof(packed_frame_size));
		return boost::endian::big_to_native(packed_frame_size);
	}

	static void
	on_read(uv_stream_t* stream_handle, ssize_t nread, const uv_buf_t* buf);

	virtual void
	really_start_read(std::error_code& err, praktor::channel::read_handler&& handler) override;

	virtual void
	really_write(mutable_buffer&& buf, std::error_code& err, praktor::channel::write_buffer_handler&& handler)
			override;

	virtual void
	really_write(
			std::deque<mutable_buffer>&&                       bufs,
			std::error_code&                                   err,
			praktor::channel::write_buffers_handler&& handler) override;

	void
	read_to_frame(ptr channel_ptr, util::const_buffer&& buf);

	bool
	is_frame_size_valid() const
	{
		return m_frame_size >= 0;
	}

	bool
	is_header_complete() const
	{
		assert(m_header_byte_count <= sizeof(frame_size_type));
		return m_header_byte_count == sizeof(frame_size_type);
	}

	std::size_t          m_header_byte_count;
	frame_size_type      m_frame_size;
	util::byte_type m_header_buf[sizeof(frame_size_type)];
	mutable_buffer       m_payload_buffer;
};

class tcp_acceptor_uv : public tcp_base_uv, public praktor::tcp_acceptor
{
public:
	using ptr = util::shared_ptr<tcp_acceptor_uv>;

	tcp_acceptor_uv()
	: m_is_framing{false}
	{}

	void
	init(uv_loop_t* lp, ptr const& self, std::error_code& err);

	virtual void
	clear_handler() override
	{
		if (m_close_handler)
		{
			m_close_handler(util::dynamic_pointer_cast<tcp_acceptor_uv>(m_data.m_self_ptr));
			m_close_handler = nullptr;
		}
		m_connection_handler = nullptr;
	}

	virtual endpoint
	get_endpoint(std::error_code& err) override
	{
		return really_get_endpoint(err);
	}

	virtual endpoint
	get_endpoint() override
	{
		std::error_code err;
		return really_get_endpoint(err);
		if (err)
		{
			throw std::system_error{err};
		}
	}

	virtual void
	set_close_handler(praktor::acceptor::close_handler&& handler) override
	{
		m_close_handler = std::move(handler);
	}

private:
	static ptr
	get_shared_acceptor(uv_stream_t* handle)
	{
		return util::dynamic_pointer_cast<tcp_acceptor_uv>(
				get_base_shared_ptr(reinterpret_cast<uv_handle_t*>(handle)));
	}

	static ptr
	get_shared_acceptor(uv_tcp_t* handle)
	{
		return util::dynamic_pointer_cast<tcp_acceptor_uv>(
				get_base_shared_ptr(reinterpret_cast<uv_handle_t*>(handle)));
	}

	static ptr
	get_shared_acceptor(uv_handle_t* handle)
	{
		return util::dynamic_pointer_cast<tcp_acceptor_uv>(get_base_shared_ptr(handle));
	}

	static tcp_acceptor_uv*
	get_acceptor_impl_raw(uv_handle_t* handle)
	{
		return util::dynamic_pointer_cast<tcp_acceptor_uv>(get_base_shared_ptr(handle)).get();
	}

	static void
	on_connection(uv_stream_t* handle, int stat);

	static void
	on_framing_connection(uv_stream_t* handle, int stat);

	virtual std::shared_ptr<praktor::loop>
	loop() override
	{
		return get_loop();
	}

	virtual bool
	really_close(praktor::acceptor::close_handler&& handler) override;

	virtual bool
	really_close() override;

	virtual void
	really_bind(praktor::options const& opts, std::error_code& err) override;

	virtual void
	really_listen(std::error_code& err, connection_handler&& handler) override;

	praktor::acceptor::connection_handler m_connection_handler;
	praktor::acceptor::close_handler      m_close_handler;
	bool	m_is_framing;
};

#endif    // PRAKTOR_TCP_UV_H
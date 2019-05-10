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

#ifndef ASYNC_TRANSCEIVER_H
#define ASYNC_TRANSCEIVER_H

#include <chrono>
#include <deque>
#include <functional>
#include <async/endpoint.h>
#include <util/buffer.h>
#include <util/shared_ptr.h>
#include <memory>
#include <system_error>

#ifndef ASYNC_TRANSCEIVER_MAX_MSG_SIZE
#define ASYNC_TRANSCEIVER_MAX_MSG_SIZE (9216)
#endif


namespace async
{

class loop;

class transceiver
{
public:
	using ptr = util::shared_ptr<transceiver>;

	using receive_handler = std::function<
			void(transceiver::ptr const& chan, util::const_buffer&& buf, ip::endpoint const& ep, std::error_code const& err)>;

	using send_buffer_handler = std::function<void(
			transceiver::ptr const& trans,
			util::mutable_buffer&&  buf,
			ip::endpoint const&     ep,
			std::error_code         err)>;

	using send_buffers_handler = std::function<void(
			transceiver::ptr const&            trans,
			std::deque<util::mutable_buffer>&& bufs,
			ip::endpoint const&                ep,
			std::error_code                    err)>;

	using close_handler = std::function<void(transceiver::ptr const& chan)>;

	static constexpr std::size_t payload_size_limit = ASYNC_TRANSCEIVER_MAX_MSG_SIZE;

	void
	start_receive(std::error_code& err, transceiver::receive_handler handler)
	{
		really_start_receive(err, std::move(handler));
	}

	void
	start_receive(transceiver::receive_handler handler)
	{
		std::error_code err;
		really_start_receive(err, std::move(handler));
		if (err)
		{
			throw std::system_error{err};
		}
	}

	virtual void
	stop_receive()
			= 0;

	void
	emit(util::mutable_buffer&& buf, ip::endpoint const& dest, std::error_code& err, send_buffer_handler handler)
	{
		really_send(std::move(buf), dest, err, std::move(handler));
	}

	void
	emit(util::mutable_buffer&& buf, ip::endpoint const& dest, send_buffer_handler handler)
	{
		std::error_code err;
		really_send(std::move(buf), dest, err, std::move(handler));
		if (err)
		{
			throw std::system_error{err};
		}
	}

	void
	emit(std::deque<util::mutable_buffer>&& bufs, ip::endpoint const& dest, std::error_code& err, send_buffers_handler handler)
	{
		really_send(std::move(bufs), dest, err, std::move(handler));
	}

	void
	emit(std::deque<util::mutable_buffer>&& bufs, ip::endpoint const& dest, send_buffers_handler handler)
	{
		std::error_code err;
		really_send(std::move(bufs), dest, err, std::move(handler));
		if (err)
		{
			throw std::system_error{err};
		}
	}

	void
	emit(util::mutable_buffer&& buf, ip::endpoint const& dest, std::error_code& err)
	{
		really_send(std::move(buf), dest, err, nullptr);
	}

	void
	emit(util::mutable_buffer&& buf, ip::endpoint const& dest)
	{
		std::error_code err;
		really_send(std::move(buf), dest, err, nullptr);
		if (err)
		{
			throw std::system_error{err};
		}
	}

	void
	write(std::deque<util::mutable_buffer>&& bufs, ip::endpoint const& dest, std::error_code& err)
	{
		really_send(std::move(bufs), dest, err, nullptr);
	}

	void
	write(std::deque<util::mutable_buffer>&& bufs, ip::endpoint const& dest)
	{
		std::error_code err;
		really_send(std::move(bufs), dest, err, nullptr);
		if (err)
		{
			throw std::system_error{err};
		}
	}

	bool
	close(close_handler handler)
	{
		return really_close(std::move(handler));
	}

	bool
	close()
	{
		return really_close(nullptr);
	}

	virtual bool
	is_closing()
			= 0;

	virtual std::shared_ptr<loop>
	loop() = 0;

protected:
	virtual void
	really_start_receive(std::error_code& err, receive_handler&& handler)
			= 0;

	virtual void
	really_send(
			util::mutable_buffer&& buf,
			ip::endpoint const&    dest,
			std::error_code&       err,
			send_buffer_handler&&  handler)
			= 0;

	virtual void
	really_send(
			std::deque<util::mutable_buffer>&& bufs,
			ip::endpoint const&                dest,
			std::error_code&                   err,
			send_buffers_handler&&             handler)
			= 0;

	virtual bool
	really_close(close_handler&& handler)
			= 0;
};

}    // namespace async

#endif    // ASYNC_TRANSCEIVER_H
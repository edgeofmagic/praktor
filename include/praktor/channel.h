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

#ifndef PRAKTOR_CHANNEL_H
#define PRAKTOR_CHANNEL_H

#include <chrono>
#include <deque>
#include <functional>
#include <praktor/endpoint.h>
#include <praktor/options.h>
#include <util/buffer.h>
#include <util/shared_ptr.h>
#include <memory>
#include <system_error>


namespace praktor
{

class loop;

class channel
{
public:
	using ptr = util::shared_ptr<channel>;

	using read_handler = std::function<void(channel::ptr const& chan, util::const_buffer&& buf, std::error_code const& err)>;

	using write_buffer_handler
			= std::function<void(channel::ptr const& chan, util::mutable_buffer&& buf, std::error_code const& err)>;

	using write_buffers_handler = std::function<
			void(channel::ptr const& chan, std::deque<util::mutable_buffer>&& bufs, std::error_code const& err)>;

	using connect_handler = std::function<void(channel::ptr const& chan, std::error_code const& err)>;

	using close_handler = std::function<void(channel::ptr const& chan)>;

	virtual ~channel() {}

	void
	start_read(std::error_code& err, read_handler handler)
	{
		really_start_read(err, std::move(handler));
	}

	void
	start_read(read_handler handler)
	{
		std::error_code err;
		really_start_read(err, std::move(handler));
		if (err)
		{
			throw std::system_error{err};
		}
	}

	virtual void
	stop_read()
			= 0;

	virtual std::shared_ptr<loop>
	loop() = 0;

	void
	write(util::mutable_buffer&& buf, std::error_code& err, write_buffer_handler handler)
	{
		really_write(std::move(buf), err, std::move(handler));
	}

	void
	write(util::mutable_buffer&& buf, write_buffer_handler handler)
	{
		std::error_code err;
		really_write(std::move(buf), err, std::move(handler));
		if (err)
		{
			throw std::system_error{err};
		}
	}

	void
	write(std::deque<util::mutable_buffer>&& bufs, std::error_code& err, write_buffers_handler handler)
	{
		really_write(std::move(bufs), err, std::move(handler));
	}

	void
	write(std::deque<util::mutable_buffer>&& bufs, write_buffers_handler handler)
	{
		std::error_code err;
		really_write(std::move(bufs), err, std::move(handler));
		if (err)
		{
			throw std::system_error{err};
		}
	}

	void
	write(util::mutable_buffer&& buf, std::error_code& err)
	{
		really_write(std::move(buf), err, nullptr);
	}

	void
	write(util::mutable_buffer&& buf)
	{
		std::error_code err;
		really_write(std::move(buf), err, nullptr);
		if (err)
		{
			throw std::system_error{err};
		}
	}

	void
	write(std::deque<util::mutable_buffer>&& bufs, std::error_code& err)
	{
		really_write(std::move(bufs), err, nullptr);
	}

	void
	write(std::deque<util::mutable_buffer>&& bufs)
	{
		std::error_code err;
		really_write(std::move(bufs), err, nullptr);
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
		return really_close();
	}

	void
	on_close(close_handler handler)
	{
		set_close_handler(std::move(handler));
	}

	virtual bool
	is_closing()
			= 0;

	virtual ip::endpoint
	get_endpoint(std::error_code& err)
			= 0;

	virtual ip::endpoint
	get_endpoint()
			= 0;

	virtual ip::endpoint
	get_peer_endpoint(std::error_code& err)
			= 0;

	virtual ip::endpoint
	get_peer_endpoint()
			= 0;

	virtual std::size_t
	get_queue_size() const = 0;

protected:
	virtual void
	set_close_handler(close_handler&& handler)
			= 0;

	virtual void
	really_write(util::mutable_buffer&& buf, std::error_code& err, write_buffer_handler&& handler)
			= 0;

	virtual void
	really_write(std::deque<util::mutable_buffer>&& bufs, std::error_code& err, write_buffers_handler&& handler)
			= 0;

	virtual bool
	really_close(close_handler&& handler)
			= 0;

	virtual bool
	really_close()
			= 0;

	virtual void
	really_start_read(std::error_code& err, read_handler&& handler)
			= 0;

};

class acceptor
{
public:
	using ptr = util::shared_ptr<acceptor>;
	using connection_handler
			= std::function<void(acceptor::ptr const& sp, channel::ptr const& chan, std::error_code const& err)>;
	using close_handler = std::function<void(acceptor::ptr const& lp)>;

	virtual ~acceptor() {}

	bool
	close(close_handler handler)
	{
		return really_close(std::move(handler));
	}

	void
	bind(praktor::options const& opts)
	{
		std::error_code err;
		really_bind(opts, err);
		if (err)
		{
			throw std::system_error{err};
		}
	}

	void
	bind(praktor::options const& opts, std::error_code& err)
	{
		really_bind(opts, err);
	}

	void
	listen(std::error_code& err, connection_handler handler)
	{
		really_listen(err, std::move(handler));
	}

	void
	listen(connection_handler handler)
	{
		std::error_code err;
		really_listen(err, std::move(handler));
		if (err)
		{
			throw std::system_error{err};
		}

	}

	bool
	close()
	{
		return really_close();
	}

	void
	on_close(close_handler handler)
	{
		set_close_handler(std::move(handler));
	}

	virtual ip::endpoint
	get_endpoint(std::error_code& err)
			= 0;

	virtual ip::endpoint
	get_endpoint()
			= 0;

	virtual std::shared_ptr<loop>
	loop() = 0;

protected:
	virtual void
	set_close_handler(close_handler&& handler)
			= 0;

	virtual bool
	really_close(close_handler&& handler)
			= 0;

	virtual bool
	really_close()
			= 0;
	
	virtual void
	really_bind(praktor::options const& opts, std::error_code& err) = 0;

	virtual void
	really_listen(std::error_code& err, connection_handler&& handler) = 0;
};

}    // namespace praktor

#endif    // PRAKTOR_CHANNEL_H
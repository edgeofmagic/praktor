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

#ifndef ASYNC_OPTIONS_H
#define ASYNC_OPTIONS_H

#include <async/endpoint.h>
#include <memory>


namespace async
{

class options
{
public:
	options(ip::endpoint const& ep)
		: m_endpoint{ep},
		  m_framing{false},
		  m_nodelay_was_set{false},
		  m_nodelay{false},
		  m_keepalive_was_set{false},
		  m_keepalive{false},
		  m_keepalive_time{std::chrono::seconds{0}}
	{}

	options(options const& rhs)
		: m_endpoint{rhs.m_endpoint},
		  m_framing{rhs.m_framing},
		  m_nodelay_was_set{rhs.m_nodelay_was_set},
		  m_nodelay{rhs.m_nodelay},
		  m_keepalive_was_set{rhs.m_keepalive_was_set},
		  m_keepalive{rhs.m_keepalive},
		  m_keepalive_time{rhs.m_keepalive_time}
	{}

	static options
	create(ip::endpoint const& ep)
	{
		return options{ep};
	}

	ip::endpoint const&
	endpoint() const
	{
		return m_endpoint;
	}

	options&
	framing(bool value)
	{
		m_framing = value;
		return *this;
	}

	bool
	framing() const
	{
		return m_framing;
	}

	options&
	nodelay(bool value)
	{
		m_nodelay         = value;
		m_nodelay_was_set = true;
		return *this;
	}

	bool
	nodelay() const
	{
		return m_nodelay;
	}

	options&
	keepalive(bool value, std::chrono::seconds period)
	{
		m_keepalive         = value;
		m_keepalive_time    = period;
		m_keepalive_was_set = true;
		return *this;
	}

	bool
	keepalive() const
	{
		return m_keepalive;
	}

	std::chrono::seconds
	keepalive_time() const
	{
		return m_keepalive_time;
	}

private:
	ip::endpoint         m_endpoint;
	bool                 m_framing;
	bool                 m_nodelay_was_set;
	bool                 m_nodelay;
	bool                 m_keepalive_was_set;
	bool                 m_keepalive;
	std::chrono::seconds m_keepalive_time;
};

}    // namespace async

#endif    // ASYNC_OPTIONS_H
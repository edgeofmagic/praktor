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

#include "timer_uv.h"
#include "loop_uv.h"
#include <iostream>

timer_uv::timer_uv(uv_loop_t* lp, std::error_code& err) : m_handler{}
{
	err.clear();;
	auto status = uv_timer_init(lp, &m_uv_timer);
	UV_ERROR_CHECK(status, err, exit);
exit:
	return;
}

timer_uv::timer_uv(uv_loop_t* lp, std::error_code& err, praktor::timer::handler handler) : m_handler{std::move(handler)}
{
	err.clear();
	auto status = uv_timer_init(lp, &m_uv_timer);
	UV_ERROR_CHECK(status, err, exit);
exit:
	return;
}

timer_uv::~timer_uv()
{
	// std::cout << "in timer_uv destructor" << std::endl;
	// assert( m_uv_timer == nullptr );
}

void
timer_uv::init(timer_uv::ptr const& self)
{
	m_data.m_impl_ptr = self;
	uv_handle_set_data(reinterpret_cast<uv_handle_t*>(&m_uv_timer), &m_data);
}

void
timer_uv::start(std::chrono::milliseconds timeout, std::error_code& err)
{
	int status = 0;
	err.clear();

	if (!m_handler)
	{
		err = make_error_code(std::errc::invalid_argument);
		goto exit;
	}

	if (is_active())
	{
		err = make_error_code(std::errc::operation_in_progress);
		goto exit;
	}

	status = uv_timer_start(&m_uv_timer, on_timer_expire, timeout.count(), 0);
	UV_ERROR_CHECK(status, err, exit);

exit:
	return;
}

void
timer_uv::start(std::chrono::milliseconds timeout)
{
	int status = 0;

	if (!m_handler)
	{
		throw std::system_error{make_error_code(std::errc::invalid_argument)};
	}

	if (is_active())
	{
		throw std::system_error{make_error_code(std::errc::operation_in_progress)};
	}

	status = uv_timer_start(&m_uv_timer, on_timer_expire, timeout.count(), 0);
	UV_ERROR_THROW(status);
}

void
timer_uv::start(std::chrono::milliseconds timeout, std::error_code& err, praktor::timer::handler handler)
{
	int status = 0;
	err.clear();

	if (!handler)
	{
		err = make_error_code(std::errc::invalid_argument);
		goto exit;
	}

	if (is_active())
	{
		err = make_error_code(std::errc::operation_in_progress);
		goto exit;
	}

	m_handler = std::move(handler);
	status = uv_timer_start(&m_uv_timer, on_timer_expire, timeout.count(), 0);
	UV_ERROR_CHECK(status, err, exit);

exit:
	return;
}

void
timer_uv::start(std::chrono::milliseconds timeout, praktor::timer::handler handler)
{
	int status = 0;

	if (!handler)
	{
		throw std::system_error{make_error_code(std::errc::invalid_argument)};
	}

	if (is_active())
	{
		throw std::system_error{make_error_code(std::errc::operation_in_progress)};
	}

	m_handler = std::move(handler);
	status = uv_timer_start(&m_uv_timer, on_timer_expire, timeout.count(), 0);
	UV_ERROR_THROW(status);
}

void
timer_uv::start(std::chrono::milliseconds timeout, std::error_code& err, praktor::timer::void_handler handler)
{
	int status = 0;
	err.clear();

	if (!handler)
	{
		err = make_error_code(std::errc::invalid_argument);
		goto exit;
	}

	if (is_active())
	{
		err = make_error_code(std::errc::operation_in_progress);
		goto exit;
	}

	m_handler = [=, handler{std::move(handler)}](praktor::timer::ptr) { handler(); };
	status = uv_timer_start(&m_uv_timer, on_timer_expire, timeout.count(), 0);
	UV_ERROR_CHECK(status, err, exit);

exit:
	return;
}

void
timer_uv::start(std::chrono::milliseconds timeout, praktor::timer::void_handler handler)
{
	int status = 0;

	if (!handler)
	{
		throw std::system_error{make_error_code(std::errc::invalid_argument)};
	}

	if (is_active())
	{
		throw std::system_error{make_error_code(std::errc::operation_in_progress)};
	}

	m_handler = [=, handler{std::move(handler)}](praktor::timer::ptr) { handler(); };
	status = uv_timer_start(&m_uv_timer, on_timer_expire, timeout.count(), 0);
	UV_ERROR_THROW(status);
}

void
timer_uv::stop(std::error_code& err)
{
	err.clear();

	if (is_active())
	{
		auto status = uv_timer_stop(&m_uv_timer);
		UV_ERROR_CHECK(status, err, exit);
	}

exit:
	return;
}

void
timer_uv::stop()
{
	if (is_active())
	{
		auto status = uv_timer_stop(&m_uv_timer);
		UV_ERROR_THROW(status);
	}
}

std::shared_ptr<praktor::loop>
timer_uv::loop()
{
	return reinterpret_cast<loop_data*>(reinterpret_cast<uv_handle_t*>(&m_uv_timer)->loop->data)->get_loop_ptr();
}

void
timer_uv::close()
{
	if (!uv_is_closing(reinterpret_cast<uv_handle_t*>(&m_uv_timer)))
	{
		uv_close(reinterpret_cast<uv_handle_t*>(&m_uv_timer), on_timer_close);
	}
}

bool
timer_uv::is_active() const
{
	return uv_is_active(reinterpret_cast<const uv_handle_t*>(&m_uv_timer));
}

bool
timer_uv::is_pending() const
{
	return is_active();
}

void
timer_uv::clear()
{
	m_handler = nullptr;          // clear handler--possibly a closure holding shared references
	m_data.m_impl_ptr.reset();    // release shared self-reference
}

void
timer_uv::on_timer_close(uv_handle_t* handle)
{
	assert(uv_handle_get_type(handle) == uv_handle_type::UV_TIMER);
	auto self_raw_ptr = reinterpret_cast<timer_handle_data*>(handle->data)->m_impl_ptr.get();
	assert(reinterpret_cast<uv_handle_t*>(&(self_raw_ptr->m_uv_timer)) == handle);
	self_raw_ptr->clear();
}

void
timer_uv::on_timer_expire(uv_timer_t* handle)
{
	timer_handle_data* const data
			= reinterpret_cast<timer_handle_data*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(handle)));
	data->m_impl_ptr->m_handler(data->m_impl_ptr);

	if (!uv_is_active(reinterpret_cast<uv_handle_t*>(handle)))
	{
		if (data->m_impl_ptr.use_count() <= 1)
		{
			// The timer is not active. If the self-reference ( in m_data ) is the only extant reference,
			// timer will become unusable and it will constitute a memory leak. Close it.
			// Closing resets the self-reference, resulting in destruction.
			// std::cout << "closing timer: low refcount" << std::endl;
			data->m_impl_ptr->close();
		}
	}
}

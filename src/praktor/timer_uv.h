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

#ifndef PRAKTOR_TIMER_UV_H
#define PRAKTOR_TIMER_UV_H

#include "uv_error.h"
#include <praktor/timer.h>
#include <uv.h>

class timer_uv;

struct timer_handle_data
{
	util::shared_ptr<timer_uv> m_impl_ptr;
};

class timer_uv : public praktor::timer
{
public:
	using ptr = util::shared_ptr<timer_uv>;

	timer_uv(uv_loop_t* lp, std::error_code& err);

	timer_uv(uv_loop_t* lp, std::error_code& err, praktor::timer::handler handler);

	virtual ~timer_uv();

	void
	init(ptr const& self);

	static void
	on_timer_close(uv_handle_t* handle);

private:
	virtual void
	start(std::chrono::milliseconds timeout, std::error_code& err) override;

	virtual void
	start(std::chrono::milliseconds timeout) override;

	virtual void
	start(std::chrono::milliseconds timeout, std::error_code& err, praktor::timer::handler handler) override;

	virtual void
	start(std::chrono::milliseconds timeout, praktor::timer::handler handler) override;

	virtual void
	start(std::chrono::milliseconds timeout, std::error_code& err, praktor::timer::void_handler handler) override;

	virtual void
	start(std::chrono::milliseconds timeout, praktor::timer::void_handler handler) override;

	virtual void
	stop(std::error_code& err) override;

	virtual void
	stop() override;

	virtual void
	close() override;

	virtual std::shared_ptr<praktor::loop>
	loop() override;

	bool
	is_active() const;

	virtual bool
	is_pending() const override;

	void
	clear();

	static void
	on_timer_expire(uv_timer_t* handle);

	uv_timer_t                       m_uv_timer;
	timer_handle_data                m_data;
	praktor::timer::handler m_handler;
};

#endif    // PRAKTOR_TIMER_UV_H
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

#ifndef PRAKTOR_TIMER_H
#define PRAKTOR_TIMER_H

#include <chrono>
#include <functional>
#include <util/shared_ptr.h>
#include <memory>
#include <system_error>


namespace praktor
{

class loop;

class timer
{
public:
	using ptr          = util::shared_ptr<timer>;
	using handler      = std::function<void(timer::ptr)>;
	using void_handler = std::function<void()>;

	virtual ~timer() {}

	virtual void
	start(std::chrono::milliseconds timeout)
			= 0;

	virtual void
	start(std::chrono::milliseconds timeout, std::error_code& err)
			= 0;

	virtual void
	start(std::chrono::milliseconds timeout, handler h)
			= 0;

	virtual void
	start(std::chrono::milliseconds timeout, std::error_code&, handler h)
			= 0;

	virtual void
	start(std::chrono::milliseconds timeout, void_handler h)
			= 0;

	virtual void
	start(std::chrono::milliseconds timeout, std::error_code& err, void_handler h)
			= 0;

	virtual void
	stop(std::error_code& err)
			= 0;

	virtual void
	stop() = 0;

	virtual void
	close() = 0;

	virtual std::shared_ptr<loop>
	loop() = 0;

	virtual bool
	is_pending() const = 0;
};

}    // namespace praktor

#endif    // PRAKTOR_TIMER_H
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

#include <praktor/loop.h>
#include <doctest.h>
#include <iostream>
#include <util/buffer.h>

#define END_LOOP(loop_ptr, delay_ms)                                                                                   \
	{                                                                                                                  \
		std::error_code _err;                                                                                          \
		auto            loop_exit_timer = loop_ptr->create_timer(_err, [](praktor::timer::ptr tp) {                      \
            std::error_code err;                                                                            \
            tp->loop()->stop(err);                                                                          \
            CHECK(!err);                                                                                    \
        });                                                                                                 \
		CHECK(!_err);                                                                                                  \
		loop_exit_timer->start(std::chrono::milliseconds{delay_ms}, _err);                                             \
		CHECK(!_err);                                                                                                  \
	}


#define DELAYED_ACTION_BEGIN(_loop_ptr)                                                                                \
	{                                                                                                                  \
		std::error_code _err;                                                                                          \
	auto _action_timer = _loop_ptr->create_timer(_err, [&](praktor::timer::ptr tp)

#define DELAYED_ACTION_END(_delay_ms)                                                                                  \
	);                                                                                                                 \
	CHECK(!_err);                                                                                                      \
	_action_timer->start(std::chrono::milliseconds{_delay_ms}, _err);                                                  \
	CHECK(!_err);                                                                                                      \
	}

using namespace praktor;

TEST_CASE("praktor::udp [ smoke ] { basic functionality }")
{
	bool acceptor_handler_did_execute{false};
	bool send_timer_did_execute{false};

	std::error_code err;
	auto            lp = loop::create();

	END_LOOP(lp, 2000);

	auto recvr = lp->create_transceiver(praktor::options{praktor::ip::endpoint{praktor::ip::address::v4_any(), 7002}}, err);
	CHECK(!err);

	recvr->start_receive(
			err,
			[&](praktor::transceiver::ptr    transp,
				util::const_buffer&&       buf,
				praktor::ip::endpoint const& ep,
				std::error_code const&     err) {
				std::cout << "in receiver handler, buffer: " << buf.to_string() << std::endl;
				std::cout << "received from " << ep.to_string() << std::endl;
			});

	praktor::transceiver::ptr trans
			= lp->create_transceiver(praktor::options{praktor::ip::endpoint{praktor::ip::address::v4_any(), 0}}, err);
	CHECK(!err);

	util::mutable_buffer msg("hello there");
	CHECK(!err);

	DELAYED_ACTION_BEGIN(lp)
	{
		trans->emit(std::move(msg), praktor::ip::endpoint{praktor::ip::address::v4_loopback(), 7002}, err);
		CHECK(!err);
		std::cout << "sending buffer on UDP socket" << std::endl;
		send_timer_did_execute = true;
	}
	DELAYED_ACTION_END(1000);

	lp->run(err);
	CHECK(!err);

	CHECK(send_timer_did_execute);

	lp->close(err);
	CHECK(!err);
}

TEST_CASE("praktor::udp [ smoke ] { error on redundant receive }")
{
	bool first_receive_handler_did_execute{false};
	bool second_receive_handler_did_execute{false};
	bool send_timer_did_execute{false};

	std::error_code err;
	auto            lp = loop::create();

	END_LOOP(lp, 2000);

	auto recvr = lp->create_transceiver(
			praktor::options{praktor::ip::endpoint{praktor::ip::address::v4_any(), 7002}},
			err,
			[&](praktor::transceiver::ptr    transp,
				util::const_buffer&&       buf,
				praktor::ip::endpoint const& ep,
				std::error_code const&     err) {
				first_receive_handler_did_execute = true;
				CHECK(buf.to_string() == "hello there");
				std::cout << "in first receiver handler, buffer: " << buf.to_string() << std::endl;
				std::cout << "received from " << ep.to_string() << std::endl;
			});
	CHECK(!err);

	recvr->start_receive(
			err,
			[&](praktor::transceiver::ptr    transp,
				util::const_buffer&&       buf,
				praktor::ip::endpoint const& ep,
				std::error_code const&     err) { second_receive_handler_did_execute = true; });
	CHECK(err);
	CHECK(err == std::errc::connection_already_in_progress);

	praktor::transceiver::ptr trans
			= lp->create_transceiver(praktor::options{praktor::ip::endpoint{praktor::ip::address::v4_any(), 0}}, err);
	CHECK(!err);

	util::mutable_buffer msg("hello there");
	CHECK(!err);

	DELAYED_ACTION_BEGIN(lp)
	{
		trans->emit(std::move(msg), praktor::ip::endpoint{praktor::ip::address::v4_loopback(), 7002}, err);
		CHECK(!err);
		send_timer_did_execute = true;
	}
	DELAYED_ACTION_END(1000);

	lp->run(err);
	CHECK(!err);

	CHECK(send_timer_did_execute);
	CHECK(first_receive_handler_did_execute);
	CHECK(!second_receive_handler_did_execute);

	lp->close(err);
	CHECK(!err);
}

TEST_CASE("praktor::udp [ smoke ] { max datagram size }")
{
	bool receive_handler_did_execute{false};
	bool send_timer_did_execute{false};

	util::mutable_buffer big{praktor::transceiver::payload_size_limit};
	big.fill(static_cast<util::byte_type>('Z'));
	big.size(praktor::transceiver::payload_size_limit);

	std::error_code err;
	auto            lp = loop::create();

	END_LOOP(lp, 2000);

	auto recvr = lp->create_transceiver(
			praktor::options{praktor::ip::endpoint{praktor::ip::address::v4_any(), 7002}},
			err,
			[&](praktor::transceiver::ptr    transp,
				util::const_buffer&&       buf,
				praktor::ip::endpoint const& ep,
				std::error_code const&     err) {
				CHECK(!err);
				receive_handler_did_execute = true;
				CHECK(buf.size() == praktor::transceiver::payload_size_limit);
				if (buf.size() == praktor::transceiver::payload_size_limit)
				{
					bool same = true;
					for (std::size_t i = 0; i < praktor::transceiver::payload_size_limit; ++i)
					{
						if (buf.data()[i] != static_cast<util::byte_type>('Z'))
						{
							same = false;
							break;
						}
					}
					CHECK(same);
				}
				std::cout << "in first receiver handler, buffer size: " << buf.size() << std::endl;
				std::cout << "received from " << ep.to_string() << std::endl;
			});
	CHECK(!err);

	praktor::transceiver::ptr trans
			= lp->create_transceiver(praktor::options{praktor::ip::endpoint{praktor::ip::address::v4_any(), 0}}, err);
	CHECK(!err);

	DELAYED_ACTION_BEGIN(lp)
	{
		trans->emit(
				std::move(big),
				praktor::ip::endpoint{praktor::ip::address::v4_loopback(), 7002},
				err,
				[&](transceiver::ptr const&    trans,
					util::mutable_buffer&&     buf,
					praktor::ip::endpoint const& ep,
					std::error_code const&     err) {
					CHECK(!err);
					std::cout << "error in emit handler, " << err.message() << std::endl;
					std::cout << "emit handler called, buf size is " << buf.size() << std::endl;
				});
		CHECK(!err);
		send_timer_did_execute = true;
	}
	DELAYED_ACTION_END(1000);

	lp->run(err);
	CHECK(!err);

	CHECK(send_timer_did_execute);
	CHECK(receive_handler_did_execute);

	lp->close(err);
	CHECK(!err);
}

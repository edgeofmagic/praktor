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

#include <doctest.h>
#include <iostream>
#include <praktor/address.h>

TEST_CASE("praktor::ip::address [ smoke ] { basic }")
{
	praktor::ip::address a;
	CHECK(a.is_v4_any());

	a = praktor::ip::address{"::"};
	CHECK(a.is_v6_any());

	std::error_code err;

	a = praktor::ip::address{"::FFFF:c0:0:2:1", err};
	CHECK(!err);
	CHECK(a.to_string() == "::ffff:c0:0:2:1");
	CHECK(a.to_uint8(0) == 0);
	CHECK(a.to_uint8(1) == 0);
	CHECK(a.to_uint8(2) == 0);
	CHECK(a.to_uint8(3) == 0);
	CHECK(a.to_uint8(4) == 0);
	CHECK(a.to_uint8(5) == 0);
	CHECK(a.to_uint8(6) == 255);
	CHECK(a.to_uint8(7) == 255);
	CHECK(a.to_uint8(8) == 0);
	CHECK(a.to_uint8(9) == 192);
	CHECK(a.to_uint8(10) == 0);
	CHECK(a.to_uint8(11) == 0);
	CHECK(a.to_uint8(12) == 0);
	CHECK(a.to_uint8(13) == 2);
	CHECK(a.to_uint8(14) == 0);
	CHECK(a.to_uint8(15) == 1);

	a = praktor::ip::address{"::FFFF:c000:201", err};
	CHECK(!err);
	CHECK(a.to_string() == "::ffff:192.0.2.1");

	a = praktor::ip::address{"1:2:3:4:0:0:0:0", err};
	CHECK(!err);
	CHECK(a.to_string() == "1:2:3:4::");

	a = praktor::ip::address{"1:2:3:4::", err};
	CHECK(!err);
	CHECK(a.to_string() == "1:2:3:4::");

	a = praktor::ip::address{"0:0:0:0:0:ffff:192.0.2.1", err};
	CHECK(!err);
	CHECK(a.to_string() == "::ffff:192.0.2.1");

	a = praktor::ip::address{"::ffff:192.0.2.1", err};
	CHECK(!err);
	CHECK(a.to_string() == "::ffff:192.0.2.1");

	a = praktor::ip::address{"0:0:0:0:0:0:192.0.2.1", err};
	CHECK(!err);
	CHECK(a.to_string() == "::192.0.2.1");
	CHECK(a.to_uint8(0) == 0);
	CHECK(a.to_uint8(1) == 0);
	CHECK(a.to_uint8(2) == 0);
	CHECK(a.to_uint8(3) == 0);
	CHECK(a.to_uint8(4) == 0);
	CHECK(a.to_uint8(5) == 0);
	CHECK(a.to_uint8(6) == 0);
	CHECK(a.to_uint8(7) == 0);
	CHECK(a.to_uint8(8) == 0);
	CHECK(a.to_uint8(9) == 0);
	CHECK(a.to_uint8(10) == 0);
	CHECK(a.to_uint8(11) == 0);
	CHECK(a.to_uint8(12) == 192);
	CHECK(a.to_uint8(13) == 0);
	CHECK(a.to_uint8(14) == 2);
	CHECK(a.to_uint8(15) == 1);

	a = praktor::ip::address{"::192.0.2.1", err};
	CHECK(!err);
	CHECK(a.to_string() == "::192.0.2.1");

	a = praktor::ip::address{"1:0:0:4:5:0:0:8", err};
	CHECK(!err);
	CHECK(a.to_string() == "1::4:5:0:0:8");

	a = praktor::ip::address{"1:0:0:4:0:0:0:8", err};
	CHECK(!err);
	CHECK(a.to_string() == "1:0:0:4::8");

	a = praktor::ip::address{"::3:0:0:0:7:8", err};
	CHECK(!err);
	CHECK(a.to_string() == "0:0:3::7:8");
}

TEST_CASE("praktor::ip::address [ smoke ] { v6_parse_errors }")
{
	praktor::ip::address a;
	CHECK(a.is_v4_any());

	std::error_code err;

	a = praktor::ip::address("::1:2:3::ff", err);
	CHECK(err);
	CHECK(err == praktor::errc::ill_formed_address);

	a = praktor::ip::address("::1:2:3:4:5:6:7", err);
	CHECK(err);
	CHECK(err == praktor::errc::ill_formed_address);

	a = praktor::ip::address("1:2:3:4:5:6:7", err);
	CHECK(err);
	CHECK(err == praktor::errc::ill_formed_address);

	a = praktor::ip::address("1:2:3:4:5:6:7:8:9", err);
	CHECK(err);
	CHECK(err == praktor::errc::ill_formed_address);

	a = praktor::ip::address("1:2:3:4:5:6:7:8::", err);
	CHECK(err);
	CHECK(err == praktor::errc::ill_formed_address);

	a = praktor::ip::address("1:2:3:4:5:6:7::", err);
	CHECK(err);
	CHECK(err == praktor::errc::ill_formed_address);

	a = praktor::ip::address("1:2:3:4::5:6:7", err);
	CHECK(err);
	CHECK(err == praktor::errc::ill_formed_address);

	a = praktor::ip::address("1:2:3::fg", err);
	CHECK(err);
	CHECK(err == praktor::errc::ill_formed_address);

	a = praktor::ip::address("::ffff:192.0.2.c0", err);
	CHECK(err);
	CHECK(err == praktor::errc::ill_formed_address);

	a = praktor::ip::address("::ffff:192.0.2", err);
	CHECK(err);
	CHECK(err == praktor::errc::ill_formed_address);

	a = praktor::ip::address("::ffff:192.0.2.1.3", err);
	CHECK(err);
	CHECK(err == praktor::errc::ill_formed_address);

	a = praktor::ip::address("::ffff:192.0.2:1", err);
	CHECK(err);
	CHECK(err == praktor::errc::ill_formed_address);

	a = praktor::ip::address("::ffff:192.0.2:1::", err);
	CHECK(err);
	CHECK(err == praktor::errc::ill_formed_address);

	// make sure err gets cleared if successful

	a = praktor::ip::address{"::FFFF:c0:0:2:1", err};
	CHECK(!err);
	CHECK(a.to_string() == "::ffff:c0:0:2:1");
}

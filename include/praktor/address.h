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

#ifndef PRAKTOR_ADDRESS_H
#define PRAKTOR_ADDRESS_H

#include <deque>
#include <functional>
#include <iostream>
#include <praktor/error.h>
#include <sstream>
#include <vector>


namespace praktor
{
namespace ip
{

class address
{
public:
	enum class family
	{
		unknown = -1,
		v4      = 0,
		v6      = 1
	};

	static const address&
	v4_loopback()
	{
		static std::uint8_t bytes[] = {127, 0, 0, 1};
		static ip::address  a(bytes);
		return a;
	}

	static const address&
	v6_loopback()
	{
		static std::uint8_t bytes[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
		static ip::address  a(bytes);
		return a;
	}

	static const address&
	v4_any()
	{
		static ip::address a(family::v4);
		return a;
	}

	static const address&
	v6_any()
	{
		static ip::address a(family::v6);
		return a;
	}

	address(family f = family::v4) : m_family(f), m_addr({{0}}) {}

	address(const char* val) : m_family(family::unknown)
	{
		memset(&m_addr, 0, sizeof(m_addr));
		from_string(val);
	}

	address(const std::string& val) : m_family(family::unknown)
	{
		memset(&m_addr, 0, sizeof(m_addr));
		from_string(val);
	}

	address(const char* val, std::error_code& err) : m_family(family::unknown)
	{
		memset(&m_addr, 0, sizeof(m_addr));
		from_string(val, err);
	}

	address(const std::string& val, std::error_code& err) : m_family(family::unknown)
	{
		memset(&m_addr, 0, sizeof(m_addr));
		from_string(val, err);
	}

	address(const address& rhs) : m_family(rhs.m_family)
	{
		std::memcpy(&m_addr, &rhs.m_addr, sizeof(m_addr));
	}

	template<class T>
	address(const T& t) : m_family(family::unknown)
	{
		memset(&m_addr, 0, sizeof(m_addr));
		assign(t);
	}

	family
	family() const
	{
		return m_family;
	}

	bool
	is_v4() const
	{
		return (m_family == family::v4);
	}

	bool
	is_v6() const
	{
		return (m_family == family::v6);
	}

	bool
	is_loopback_v4() const
	{
		return is_v4() && (m_addr.m_b[0] == 127) && (m_addr.m_b[1] == 0) && (m_addr.m_b[2] == 0)
			   && (m_addr.m_b[3] == 1);
	}

	bool
	is_loopback_v6() const
	{
		return is_v6() && (m_addr.m_l[0] == 0) && (m_addr.m_l[1] == 0) && (m_addr.m_l[2] == 0) && (m_addr.m_b[12] == 0)
			   && (m_addr.m_b[13] == 0) && (m_addr.m_b[14] == 0) && (m_addr.m_b[15] == 1);
	}

	bool
	is_loopback() const
	{
		return (is_loopback_v4() || is_loopback_v6());
	}

	bool
	is_link_local_v4() const
	{
		return is_v4() && (m_addr.m_b[0] == 169) && (m_addr.m_b[1] == 254);
	}

	bool
	is_link_local_v6() const
	{
		return is_v6() && (m_addr.m_b[0] == 0xfe) && ((m_addr.m_b[1] & 0xc0) == 0x80);
	}

	bool
	is_link_local() const
	{
		return (is_link_local_v4() || is_link_local_v6());
	}

	bool
	is_multicast_v4() const
	{
		return is_v4() && ((m_addr.m_b[0] >= 224) && (m_addr.m_b[0] < 239));
	}

	bool
	is_multicast_v6() const
	{
		return is_v6() && (m_addr.m_b[0] == 0xff);
	}

	bool
	is_multicast() const
	{
		return (is_multicast_v4() || is_multicast_v6());
	}

	bool
	is_v4_any() const
	{
		return is_v4() && (m_addr.m_l[0] == 0);
	}

	bool
	is_v6_any() const
	{
		return is_v6() && (m_addr.m_l[0] == 0) && (m_addr.m_l[1] == 0) && (m_addr.m_l[2] == 0) && (m_addr.m_l[3] == 0);
	}

	bool
	is_any() const
	{
		return (is_v4_any() || is_v6_any());
	}

	std::string
	to_string() const
	{
		std::ostringstream os;

		if (is_v4())
		{
			to_v4(os);
		}
		else if (is_v6())
		{
			to_v6(os);
		}

		return os.str();
	}

	std::string
	to_reverse_string() const
	{
		std::ostringstream os;

		if (is_v4())
		{
			to_reverse_v4(os);
		}
		else if (is_v6())
		{
			to_reverse_v6(os);
		}

		return os.str();
	}

	address&
	operator=(const address& rhs)
	{
		m_family = rhs.m_family;
		std::memcpy(&m_addr, &rhs.m_addr, sizeof(m_addr));
		return *this;
	}

	template<class T>
	address&
	operator=(const T& t)
	{
		assign(t);
		return *this;
	}

	template<class T>
	address&
	operator<<(const T& t)
	{
		assign(t);
		return *this;
	}

	template<class T>
	std::enable_if_t<(sizeof(T) == 4 || sizeof(T) == 16), const address&>
	operator>>(T& t) const
	{
		memcpy(&t, &m_addr, sizeof(t));
		return *this;
	}

	address operator&(const address& rhs) const
	{
		address a;

		if (m_family == rhs.m_family)
		{
			a.m_family      = m_family;
			a.m_addr.m_l[0] = m_addr.m_l[0] & rhs.m_addr.m_l[0];
			a.m_addr.m_l[1] = m_addr.m_l[1] & rhs.m_addr.m_l[1];
			a.m_addr.m_l[2] = m_addr.m_l[2] & rhs.m_addr.m_l[2];
			a.m_addr.m_l[3] = m_addr.m_l[3] & rhs.m_addr.m_l[3];
		}

		return a;
	}

	bool
	operator==(const address& rhs) const
	{
		return (m_family == rhs.m_family) && (memcmp(&m_addr, &rhs.m_addr, sizeof(m_addr)) == 0);
	}

	bool
	operator!=(const address& rhs) const
	{
		return !(*this == rhs);
	}

	std::uint8_t
	to_uint8(std::size_t index) const
	{
		if (index < 16)
		{
			return m_addr.m_b[index];
		}
		else
		{
			return 0;
		}
	}

	std::uint16_t
	to_uint16(std::size_t index) const
	{
		if (index < 8)
		{
			return m_addr.m_s[index];
		}
		else
		{
			return 0;
		}
	}

	std::uint32_t
	to_uint32(std::size_t index) const
	{
		if (index < 4)
		{
			return m_addr.m_l[index];
		}
		else
		{
			return 0;
		}
	}

	explicit operator bool() const
	{
		return (m_addr.m_l[0] || m_addr.m_l[1] || m_addr.m_l[2] || m_addr.m_l[3]);
	}

protected:
	void
	from_string(std::string const& s);

	void
	from_string(std::string const& s, std::error_code& err);

	void
	from_v4(std::string const& s);

	// void
	// from_v4( std::string const& s, std::error_code& err );

	void
	from_v6(std::string const& s);

	void
	from_v6(std::string const& s, std::error_code& err);

	void
	to_v4(std::ostream& os) const;

	void
	to_reverse_v4(std::ostream& os) const;

	void
	to_v6(std::ostream& os) const;

	void
	to_reverse_v6(std::ostream& os) const;

	template<class T>
	std::enable_if_t<(sizeof(T) == 4 || sizeof(T) == 16)>
	assign(const T& t)
	{
		m_family = (sizeof(t) == 4) ? family::v4 : family::v6;
		std::memcpy(&m_addr, &t, sizeof(t));
	}

	enum family m_family;

	union addr_t
	{
		std::uint8_t  m_b[16];
		std::uint16_t m_s[8];
		std::uint32_t m_l[4];
	} m_addr;

	static_assert(sizeof(addr_t) == 16, "m_addr not 128 bits");
};

inline std::ostream&
operator<<(std::ostream& os, const enum ip::address::family fam)
{
	switch (fam)
	{
		case ip::address::family::v4:
			os << "ipv4";
			break;

		case ip::address::family::v6:
			os << "ipv6";
			break;

		default:
			os << "unknown";
			break;
	}

	return os;
}

}    // namespace ip
}    // namespace praktor

namespace std
{

template<>
struct hash<praktor::ip::address>
{
	typedef praktor::ip::address argument_type;
	typedef std::size_t                   result_type;

	result_type
	operator()(const argument_type& v) const
	{
		result_type val = std::hash<std::uint32_t>()(v.to_uint32(0));
		result_type res = val;

		if (v.is_v6())
		{
			val = (std::hash<std::uint32_t>()(v.to_uint32(1)));
			res = (res << 1) + res + val;
			val = (std::hash<std::uint32_t>()(v.to_uint32(2)));
			res = (res << 1) + res + val;
			val = (std::hash<std::uint32_t>()(v.to_uint32(3)));
			res = (res << 1) + res + val;
		}

		return res;
	}
};

}    // namespace std

#endif    // PRAKTOR_ADDRESS_H

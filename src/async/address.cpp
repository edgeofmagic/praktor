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

#include <algorithm>
#include <boost/endian/conversion.hpp>
#include <cctype>
#include <cstring>
#include <async/address.h>
#include <sstream>
#include <vector>

using namespace async;

void
ip::address::to_v4(std::ostream& os) const
{
	os << static_cast<std::uint32_t>(m_addr.m_b[0]) << '.' << static_cast<std::uint32_t>(m_addr.m_b[1]) << '.'
	   << static_cast<std::uint32_t>(m_addr.m_b[2]) << '.' << static_cast<std::uint32_t>(m_addr.m_b[3]);
}


void
ip::address::to_reverse_v4(std::ostream& os) const
{
	os << static_cast<std::uint32_t>(m_addr.m_b[3]) << '.' << static_cast<std::uint32_t>(m_addr.m_b[2]) << '.'
	   << static_cast<std::uint32_t>(m_addr.m_b[1]) << '.' << static_cast<std::uint32_t>(m_addr.m_b[0]);
}


void
ip::address::to_v6(std::ostream& os) const
{
	struct
	{
		int base, len;
	} best, cur;

	std::uint16_t words[] = {0, 0, 0, 0, 0, 0, 0, 0};

	for (auto i = 0; i < 8; ++i)
	{
		words[i] = boost::endian::big_to_native(m_addr.m_s[i]);
		// words[ i ] = m_addr.m_s[ i ];
	}

	best.base = -1;
	cur.base  = -1;
	best.len  = 0;
	cur.len   = 0;

	for (auto i = 0; i < 8; i++)
	{
		if (words[i] == 0)
		{
			if (cur.base == -1)
			{
				cur.base = i, cur.len = 1;
			}
			else
			{
				cur.len++;
			}
		}
		else
		{
			if (cur.base != -1)
			{
				if (best.base == -1 || cur.len > best.len)
				{
					best = cur;
				}
				cur.base = -1;
			}
		}
	}

	if (cur.base != -1)
	{
		if (best.base == -1 || cur.len > best.len)
		{
			best = cur;
		}
	}

	if (best.base != -1 && best.len < 2)
	{
		best.base = -1;
	}

	/*
	* Format the result.
	*/

	for (auto i = 0; i < 8; i++)
	{
		/* Are we inside the best run of 0x00's? */
		if (best.base != -1 && i >= best.base && i < (best.base + best.len))
		{
			if (i == best.base)
			{
				os << ':';
			}
			continue;
		}

		/* Are we following an initial run of 0x00s or any real hex? */
		if (i != 0)
		{
			os << ':';
		}

		/* Is this address an encapsulated IPv4? */
		if (i == 6 && best.base == 0 && (best.len == 6 || (best.len == 5 && words[5] == 0xffff)))
		{
			// std::cout << std::endl;
			// std::cout << "[" << static_cast< int >( m_addr.m_b[ 12 ] ) << ".";
			// std::cout << static_cast< int >( m_addr.m_b[ 13 ] ) << ".";
			// std::cout << static_cast< int >( m_addr.m_b[ 14 ] ) << ".";
			// std::cout << static_cast< int >( m_addr.m_b[ 15 ] ) << "]" << std::endl; std::cout.flush();
			//			os << m_addr.m_b[ 15 ] << '.' << m_addr.m_b[ 14 ] << m_addr.m_b[ 13 ] << '.' << m_addr.m_b[ 12 ];
			os << static_cast<unsigned>(m_addr.m_b[12]) << '.' << static_cast<unsigned>(m_addr.m_b[13]) << '.'
			   << static_cast<unsigned>(m_addr.m_b[14]) << '.' << static_cast<unsigned>(m_addr.m_b[15]);
			break;
		}

		os << std::hex << words[i] << std::dec;
	}

	/* Was it a trailing run of 0x00's? */
	if (best.base != -1 && (best.base + best.len) == 8)
	{
		os << ':';
	}
}


void
ip::address::to_reverse_v6(std::ostream& os) const
{
	for (auto i = 15; i >= 0; i--)
	{
		os << m_addr.m_b[i] << ".";
	}

	os << "ip6.arpa";
}


/* int
 * inet_pton4(src, dst)
 *	like inet_aton() but without all the hexadecimal, octal (with the
 *	exception of 0) and shorthand.
 * return:
 *	1 if `src' is a valid dotted quad, else 0.
 * notice:
 *	does not touch `dst' unless it's returning 1.
 * author:
 *	Paul Vixie, 1996.
 */
void
ip::address::from_string(const std::string& s)
{
	if (s.find(':') == std::string::npos)
	{
		from_v4(s);
	}
	else
	{
		from_v6(s);
	}
}

void
ip::address::from_string(const std::string& s, std::error_code& err)
{
	err.clear();
	if (s.find(':') == std::string::npos)
	{
		from_v4(s);
	}
	else
	{
		from_v6(s, err);
	}
}

#define NS_INT16SZ 2
#define NS_INADDRSZ 4
#define NS_IN6ADDRSZ 16
#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

void
ip::address::from_v4(const std::string& s)
{
	int    saw_digit, octets, ch;
	u_char tmp[NS_INADDRSZ], *tp;
	char*  src = const_cast<char*>(s.c_str());

	saw_digit   = 0;
	octets      = 0;
	*(tp = tmp) = 0;
	while ((ch = *src++) != '\0')
	{

		if (ch >= '0' && ch <= '9')
		{
			u_int new_int = *tp * 10 + (ch - '0');

			if (saw_digit && *tp == 0)
				goto exit;
			if (new_int > 255)
				goto exit;
			*tp = new_int;
			if (!saw_digit)
			{
				if (++octets > 4)
					goto exit;
				saw_digit = 1;
			}
		}
		else if (ch == '.' && saw_digit)
		{
			if (octets == 4)
				goto exit;
			*++tp     = 0;
			saw_digit = 0;
		}
		else
			goto exit;
	}
	if (octets < 4)
		goto exit;
	m_family = family::v4;
	memcpy(&m_addr, tmp, NS_INADDRSZ);

exit:

	return;
}


class ipv6_addr_util
{
private:
	enum class addr_elem_type
	{
		zero_compression,
		unsigned_16_bits
	};

	struct addr_elem
	{
		addr_elem(addr_elem_type t, std::uint16_t v) : m_type{t}, m_value{v} {}

		addr_elem_type m_type;
		std::uint16_t  m_value;
	};

	static bool
	is_hex_digit(char ch)
	{
		return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
	}

	static bool
	is_dec_digit(char ch)
	{
		return ch >= '0' && ch <= '9';
	}

	static std::uint16_t
	convert_hex_word(std::string const& s, std::error_code& err)
	{
		err.clear();
		unsigned long result = 0;
		try
		{
			result = stoul(s, nullptr, 16);
		}
		catch (std::exception const& ex)
		{
			err = make_error_code(async::errc::ill_formed_address);
		}
		return boost::endian::native_to_big(static_cast<std::uint16_t>(result));
	}

	static std::uint16_t
	convert_dec_byte(std::string const& s, std::error_code& err)
	{
		err.clear();
		unsigned long result = 0;
		try
		{
			result = stoul(s);
		}
		catch (std::exception const& ex)
		{
			err = make_error_code(async::errc::ill_formed_address);
			goto exit;
		}
		if (result > 255)
		{
			err = make_error_code(async::errc::ill_formed_address);
		}
	exit:
		return static_cast<std::uint16_t>(result);
	}

	enum class parse_state
	{
		initial,
		found_initial_colon,
		found_0_compr,
		found_hex_digit,
		found_trailing_colon,
		found_dot,
		found_dec_digit
	};

public:
	union v6_address
	{
		std::uint8_t  as_u8s[16];
		std::uint16_t as_u16s[8];
		std::uint32_t as_u32s[4];
	};

	static void
	string_to_v6_addr(std::string const& s, v6_address& addr, std::error_code& err)
	{
		::memset(&addr, 0, sizeof(addr));

		std::vector<addr_elem>     elements;
		std::vector<std::uint16_t> v4_bytes;

		elements.reserve(8);
		v4_bytes.reserve(4);

		err.clear();

		std::size_t i = 0;
		std::string token;
		char        nextch;
		parse_state state       = parse_state::initial;
		bool        parse_error = false;
		bool        finished    = false;

		while (!finished && !parse_error)
		{
			if (i < s.size())
			{
				nextch = s[i];
				++i;
			}
			else
			{
				nextch = 0;
			}

			switch (state)
			{
				case parse_state::initial:
					// expect first colon of initial zero compression or first digit of value
					if (nextch == ':')
					{
						token.clear();
						token += nextch;
						state = parse_state::found_initial_colon;
					}
					else if (is_hex_digit(nextch))
					{
						token.clear();
						token += static_cast<char>(::tolower(nextch));
						state = parse_state::found_hex_digit;
					}
					else
					{
						parse_error = true;
					}
					break;

				case parse_state::found_initial_colon:
					// expect second colon of initial zero compression
					if (nextch == ':')
					{
						token += nextch;
						elements.emplace_back(addr_elem_type::zero_compression, 0);
						state = parse_state::found_0_compr;
					}
					else
					{
						parse_error = true;
					}
					break;

				case parse_state::found_0_compr:
					// expect initial digit of value or empty
					if (is_hex_digit(nextch))
					{
						token.clear();
						token += nextch;
						state = parse_state::found_hex_digit;
					}
					else if (nextch == 0)
					{
						finished = true;
					}
					else
					{
						parse_error = true;
					}
					break;

				case parse_state::found_hex_digit:    // in number
					// expect next digit of value or separator or empty
					if (is_hex_digit(nextch))
					{
						token += nextch;
					}
					else if (nextch == ':')
					{
						std::uint16_t value = convert_hex_word(token, err);
						if (err)
							parse_error = true;
						elements.emplace_back(addr_elem_type::unsigned_16_bits, value);
						state = parse_state::found_trailing_colon;
					}
					else if (nextch == '.')
					{
						std::uint16_t value = convert_dec_byte(token, err);
						if (err)
							parse_error = true;
						v4_bytes.push_back(value);
						state = parse_state::found_dot;
					}
					else if (nextch == 0)
					{
						std::uint16_t value = convert_hex_word(token, err);
						if (err)
							parse_error = true;
						elements.emplace_back(addr_elem_type::unsigned_16_bits, value);
						finished = true;
					}
					else
					{
						parse_error = true;
					}
					break;

				case parse_state::found_trailing_colon:
					// expect second colon of zero compression or initial digit of value
					if (nextch == ':')
					{
						elements.emplace_back(addr_elem_type::zero_compression, 0);
						state = parse_state::found_0_compr;
					}
					else if (is_hex_digit(nextch))
					{
						token.clear();
						token += nextch;
						state = parse_state::found_hex_digit;
					}
					else
					{
						parse_error = true;
					}
					break;

				case parse_state::found_dot:
					// expect deximal digit
					if (is_dec_digit(nextch))
					{
						token.clear();
						token += nextch;
						state = parse_state::found_dec_digit;
					}
					else
					{
						parse_error = true;
					}
					break;

				case parse_state::found_dec_digit:
					// expect next digit of decimal number in ipv4 address or dot or empty
					if (is_dec_digit(nextch))
					{
						token += nextch;
					}
					else if (nextch == '.')
					{
						std::uint16_t value = convert_dec_byte(token, err);
						if (err)
							parse_error = true;
						v4_bytes.push_back(value);
						state = parse_state::found_dot;
					}
					else if (nextch == 0)
					{
						std::uint16_t value = convert_dec_byte(token, err);
						if (err)
							parse_error = true;
						v4_bytes.push_back(value);
						finished = true;
					}
					else
					{
						parse_error = true;
					}
					break;
			}
		}


		std::size_t zero_group_count = 0;
		std::size_t value_count      = 0;
		std::size_t zero_group_size  = 0;


		if (parse_error)
		{
			goto error_exit;
		}

		if (v4_bytes.size() > 0)
		{
			if (v4_bytes.size() != 4)
			{
				err = make_error_code(async::errc::ill_formed_address);
				goto error_exit;
			}

			std::uint16_t value = (v4_bytes[1] << 8) | v4_bytes[0];
			elements.emplace_back(addr_elem_type::unsigned_16_bits, value);

			value = (v4_bytes[3] << 8) | v4_bytes[2];
			elements.emplace_back(addr_elem_type::unsigned_16_bits, value);
		}

		for (auto& part : elements)
		{
			if (part.m_type == addr_elem_type::zero_compression)
			{
				++zero_group_count;
			}
		}

		if (zero_group_count > 1)
		{
			goto error_exit;
		}

		value_count = elements.size() - zero_group_count;
		if (value_count > 8)
		{
			goto error_exit;
		}


		if (zero_group_count > 0)    // must be 1
		{
			zero_group_size = 8 - value_count;
			if (zero_group_size <= 1)
			{
				goto error_exit;
			}
		}

		if (value_count + zero_group_size != 8)
		{
			goto error_exit;
		}

		for (std::size_t addr_idx = 0, i = 0; i < elements.size(); ++i)
		{
			if (elements[i].m_type == addr_elem_type::zero_compression)
			{
				for (auto j = 0; j < zero_group_size; ++j)
				{
					addr.as_u16s[addr_idx] = 0;
					++addr_idx;
				}
			}
			else
			{
				addr.as_u16s[addr_idx] = elements[i].m_value;
				++addr_idx;
			}
		}

		return;

	error_exit:
		err = make_error_code(async::errc::ill_formed_address);
		return;
	}
};

void
ip::address::from_v6(std::string const& s, std::error_code& err)
{
	m_family = family::v6;
	ipv6_addr_util::string_to_v6_addr(s, reinterpret_cast<ipv6_addr_util::v6_address&>(m_addr), err);
}

void
ip::address::from_v6(std::string const& s)
{
	std::error_code err;
	from_v6(s, err);
}

/* int
 * inet_pton6(src, dst)
 *	convert presentation level address to network order binary form.
 * return:
 *	1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *	(1) does not touch `dst' unless it's returning 1.
 *	(2) :: in a full address is silently ignored.
 * credit:
 *	inspired by Mark Andrews.
 * author:
 *	Paul Vixie, 1996.
 */

#if 0
void
ip::address::from_v6( const std::string &s )
{
	static const char xdigits[] = "0123456789abcdef";
	u_char tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
	const char *curtok;
	int ch, saw_xdigit;
	u_int val;
	char * src = const_cast< char* >( s.c_str() );

	memset(tmp, '\0', NS_IN6ADDRSZ);
	tp = tmp;
	endp = tp + NS_IN6ADDRSZ;
	colonp = NULL;
	/* Leading :: requires some special handling. */
	if (*src == ':')
		if (*++src != ':')
			goto exit;
	curtok = src;
	saw_xdigit = 0;
	val = 0;
	while ((ch = tolower (*src++)) != '\0') {
		const char *pch;

		pch = strchr(xdigits, ch);
		if (pch != NULL) {
			val <<= 4;
			val |= (pch - xdigits);
			if (val > 0xffff)
				goto exit;
			saw_xdigit = 1;
			continue;
		}
		if (ch == ':') {
			curtok = src;
			if (!saw_xdigit) {
				if (colonp)
					goto exit;
				colonp = tp;
				continue;
			} else if (*src == '\0') {
				goto exit;
			}
			if (tp + NS_INT16SZ > endp)
				goto exit;
			*tp++ = (u_char) (val >> 8) & 0xff;
			*tp++ = (u_char) val & 0xff;
			saw_xdigit = 0;
			val = 0;
			continue;
		}
		if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) )
		{
#if 0
		    inet_pton4(curtok, tp) > 0) {
			tp += NS_INADDRSZ;
			saw_xdigit = 0;
			break;	/* '\0' was seen by inet_pton4(). */
#endif
		}
		
		goto exit;
	}
	if (saw_xdigit) {
		if (tp + NS_INT16SZ > endp)
			goto exit;
		*tp++ = (u_char) (val >> 8) & 0xff;
		*tp++ = (u_char) val & 0xff;
	}
	if (colonp != NULL) {
		/*
		 * Since some memmove()'s erroneously fail to handle
		 * overlapping regions, we'll do the shift by hand.
		 */
		const long n = static_cast< const long >( tp - colonp );
		int i;

		if (tp == endp)
			goto exit;
		for (i = 1; i <= n; i++) {
			endp[- i] = colonp[n - i];
			colonp[n - i] = 0;
		}
		tp = endp;
	}
	if (tp != endp)
		goto exit;
	
	m_family = family::v6;
	memcpy( &m_addr, tmp, NS_IN6ADDRSZ);
	
exit:

	return;
}
#endif
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

#include <praktor/error.h>


class address_info_category_impl : public std::error_category
{
public:
	virtual const char* 
	name() const noexcept override;

	virtual std::string 
	message( int ev ) const noexcept override;

};

const char* 
address_info_category_impl::name() const noexcept
{
    return "address_info";
}

std::string 
address_info_category_impl::message(int ev ) const noexcept
{
    switch ( static_cast< address_info::errc > (ev ) )
    {
		case address_info::errc::ok:
			return "success";
		case address_info::errc::address_family_not_supported:				// EAI_ADDRFAMILY
			return "address family for hostname not supported";
		case address_info::errc::temporary_failure:							// EAI_AGAIN
			return "temporary failure in name resolution";
		case address_info::errc::invalid_value_for_ai_flags:				// EAI_BADFLAGS
			return "invalid value for ai_flags";
		case address_info::errc::invalid_value_for_hints:					// EAI_BADHINTS
			return "invalid value for hints";
		case address_info::errc::request_canceled:							// EAI_CANCELED
			return "request canceled";
		case address_info::errc::non_recoverable_error:						// EAI_FAIL
			return "non-recoverable error in name resolution";
		case address_info::errc::ai_family_not_supported:					// EAI_FAMILY
			return "ai_family not supported";
		case address_info::errc::memory_allocation_failure:					// EAI_MEMORY
			return "memory allocation failure";
		case address_info::errc::no_address_for_name:						// EAI_NODATA
			return "no address associated with hostname";
		case address_info::errc::name_is_unknown:							// EAI_NONAME
			return "hostname or service name is unknown";
		case address_info::errc::argument_buffer_overflow:					// EAI_OVERFLOW
			return "argument buffer overflow";
		case address_info::errc::unknown_protocol:							// EAI_PROTOCOL
			return "resolved protocol is unknown";
		case address_info::errc::service_not_supported:						// EAI_SERVICE
			return "service not supported for ai_socktype";
		case address_info::errc::ai_socktype_not_supported:					// EAI_SOCKTYPE
			return "ai_socktype not supported";
		default:
			return "unknown address_info error";
    }
}

std::error_category const&
address_info::error_category() noexcept
{
    static address_info_category_impl instance;
    return instance;
}

std::error_condition 
address_info::make_error_condition( address_info::errc e )
{
    return std::error_condition( static_cast< int >( e ), address_info::error_category() );
}

std::error_code 
address_info::make_error_code( address_info::errc e )
{
    return std::error_code( static_cast< int >( e ), address_info::error_category() );
}


class praktor_category_impl : public std::error_category
{
public:
	virtual const char* 
	name() const noexcept override;

	virtual std::string 
	message( int ev ) const noexcept override;

};

const char* 
praktor_category_impl::name() const noexcept
{
    return "praktor";
}

std::string 
praktor_category_impl::message(int ev ) const noexcept
{
    switch ( static_cast< praktor::errc > (ev ) )
    {
		case praktor::errc::ok:
			return "success";
		case praktor::errc::invalid_unicode_character:			// UV__ECHARSET
			return "invalid unicode character";
		case praktor::errc::host_is_down:							// UV__EHOSTDOWN
			return "host is down";
		case praktor::errc::no_network_connection:				// UV__ENONET
			return "machine not connected to network";
		case praktor::errc::end_of_file:							// UV__EOF
			return "end of file or stream";
		case praktor::errc::remote_io_error:						// UV__EREMOTEIO
			return "remote I/O error";
		case praktor::errc::transport_endpoint_shutdown:			// UV__ESHUTDOWN
			return "cannot send after transport endpoint shutdown";
		case praktor::errc::unknown_error:						// UV__UNKNOWN
			return "unknown system error";
		case praktor::errc::ill_formed_address:
			return "ill-formed address";
		case praktor::errc::loop_closed:
			return "loop closed";
		case praktor::errc::timer_closed:
			return "timer closed";
		default:
			return "unknown praktor error";
    }
}

std::error_category const&
praktor::error_category() noexcept
{
    static praktor_category_impl instance;
    return instance;
}

std::error_condition 
praktor::make_error_condition( praktor::errc e )
{
    return std::error_condition( static_cast< int >( e ), praktor::error_category() );
}

std::error_code 
praktor::make_error_code( praktor::errc e )
{
    return std::error_code( static_cast< int >( e ), praktor::error_category() );
}


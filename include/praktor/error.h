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

#ifndef PRAKTOR_ERROR_H
#define PRAKTOR_ERROR_H

#include <system_error>


namespace address_info
{

enum class errc
{
	ok = 0,
	address_family_not_supported,    // EAI_ADDRFAMILY
	temporary_failure,               // EAI_AGAIN
	invalid_value_for_ai_flags,      // EAI_BADFLAGS
	invalid_value_for_hints,         // EAI_BADHINTS
	request_canceled,                // EAI_CANCELED
	non_recoverable_error,           // EAI_FAIL
	ai_family_not_supported,         // EAI_FAMILY
	memory_allocation_failure,       // EAI_MEMORY
	no_address_for_name,             // EAI_NODATA
	name_is_unknown,                 // EAI_NONAME
	argument_buffer_overflow,        // EAI_OVERFLOW
	unknown_protocol,                // EAI_PROTOCOL
	service_not_supported,           // EAI_SERVICE
	ai_socktype_not_supported        // EAI_SOCKTYPE
};

std::error_category const&
error_category() noexcept;

std::error_condition
make_error_condition(errc e);

std::error_code
make_error_code(errc e);

}    // namespace address_info

namespace praktor
{

enum class errc
{
	ok = 0,
	invalid_unicode_character,      // UV_ECHARSET
	host_is_down,                   // UV_EHOSTDOWN
	no_network_connection,          // UV_ENONET
	end_of_file,                    // UV_EOF
	remote_io_error,                // UV_EREMOTEIO
	transport_endpoint_shutdown,    // UV_ESHUTDOWN
	unknown_error,                  // UV_UNKNOWN
	ill_formed_address,
	loop_closed,
	timer_closed,
};

std::error_category const&
error_category() noexcept;

std::error_condition
make_error_condition(errc e);

std::error_code
make_error_code(errc e);

}    // namespace praktor

namespace std
{

template<>
struct is_error_condition_enum<praktor::errc> : public true_type
{};

template<>
struct is_error_condition_enum<address_info::errc> : public true_type
{};

}    // namespace std

#endif    // PRAKTOR_ERROR_H
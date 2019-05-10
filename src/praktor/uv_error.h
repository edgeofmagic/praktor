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

#ifndef PRAKTOR_UV_ERROR_H
#define PRAKTOR_UV_ERROR_H

#include <praktor/error.h>
#include <uv.h>

#define UV_ERROR_CHECK(_uv_err_, _err_var_, _label_)                                                                   \
	{                                                                                                                  \
		if (_uv_err_ < 0)                                                                                              \
		{                                                                                                              \
			_err_var_ = map_uv_error(_uv_err_);                                                                        \
			goto _label_;                                                                                              \
		}                                                                                                              \
	}                                                                                                                  \
	/**/

#define UV_ERROR_THROW(_uv_err_)                                                                                       \
	{                                                                                                                  \
		if (_uv_err_ < 0)                                                                                              \
		{                                                                                                              \
			throw std::system_error{map_uv_error(_uv_err_)};                                                           \
		}                                                                                                              \
	}                                                                                                                  \
	/**/

inline std::error_code const&
map_uv_error(int uv_code)
{
	switch (uv_code)
	{
		case 0:
		{
			static const std::error_code success;
			return success;
		}
		case UV_E2BIG:
		{
			static const std::error_code e = make_error_code(std::errc::argument_list_too_long);
			return e;
		}
		case UV_EACCES:
		{
			static const std::error_code e = make_error_code(std::errc::permission_denied);
			return e;
		}
		case UV_EADDRINUSE:
		{
			static const std::error_code e = make_error_code(std::errc::address_in_use);
			return e;
		}
		case UV_EADDRNOTAVAIL:
		{
			static const std::error_code e = make_error_code(std::errc::address_not_available);
			return e;
		}
		case UV_EAFNOSUPPORT:
		{
			static const std::error_code e = make_error_code(std::errc::address_family_not_supported);
			return e;
		}
		case UV_EAGAIN:
		{
			static const std::error_code e = make_error_code(std::errc::resource_unavailable_try_again);
			return e;
		}
		case UV_EAI_ADDRFAMILY:
		{
			static const std::error_code e
					= make_error_code(address_info::errc::address_family_not_supported);
			return e;
		}
		case UV_EAI_AGAIN:
		{
			static const std::error_code e = make_error_code(address_info::errc::temporary_failure);
			return e;
		}
		case UV_EAI_BADFLAGS:
		{
			static const std::error_code e = make_error_code(address_info::errc::invalid_value_for_ai_flags);
			return e;
		}
		case UV_EAI_BADHINTS:
		{
			static const std::error_code e = make_error_code(address_info::errc::invalid_value_for_hints);
			return e;
		}
		case UV_EAI_CANCELED:
		{
			static const std::error_code e = make_error_code(address_info::errc::request_canceled);
			return e;
		}
		case UV_EAI_FAIL:
		{
			static const std::error_code e = make_error_code(address_info::errc::non_recoverable_error);
			return e;
		}
		case UV_EAI_FAMILY:
		{
			static const std::error_code e = make_error_code(address_info::errc::ai_family_not_supported);
			return e;
		}
		case UV_EAI_MEMORY:
		{
			static const std::error_code e = make_error_code(address_info::errc::memory_allocation_failure);
			return e;
		}
		case UV_EAI_NODATA:
		{
			static const std::error_code e = make_error_code(address_info::errc::no_address_for_name);
			return e;
		}
		case UV_EAI_NONAME:
		{
			static const std::error_code e = make_error_code(address_info::errc::name_is_unknown);
			return e;
		}
		case UV_EAI_OVERFLOW:
		{
			static const std::error_code e = make_error_code(address_info::errc::argument_buffer_overflow);
			return e;
		}
		case UV_EAI_PROTOCOL:
		{
			static const std::error_code e = make_error_code(address_info::errc::unknown_protocol);
			return e;
		}
		case UV_EAI_SERVICE:
		{
			static const std::error_code e = make_error_code(address_info::errc::service_not_supported);
			return e;
		}
		case UV_EAI_SOCKTYPE:
		{
			static const std::error_code e = make_error_code(address_info::errc::ai_socktype_not_supported);
			return e;
		}
		case UV_EALREADY:
		{
			static const std::error_code e = make_error_code(std::errc::connection_already_in_progress);
			return e;
		}
		case UV_EBADF:
		{
			static const std::error_code e = make_error_code(std::errc::bad_file_descriptor);
			return e;
		}
		case UV_EBUSY:
		{
			static const std::error_code e = make_error_code(std::errc::device_or_resource_busy);
			return e;
		}
		case UV_ECANCELED:
		{
			static const std::error_code e = make_error_code(std::errc::operation_canceled);
			return e;
		}
		case UV_ECHARSET:
		{
			static const std::error_code e = make_error_code(praktor::errc::invalid_unicode_character);
			return e;
		}
		case UV_ECONNABORTED:
		{
			static const std::error_code e = make_error_code(std::errc::connection_aborted);
			return e;
		}
		case UV_ECONNREFUSED:
		{
			static const std::error_code e = make_error_code(std::errc::connection_refused);
			return e;
		}
		case UV_ECONNRESET:
		{
			static const std::error_code e = make_error_code(std::errc::connection_reset);
			return e;
		}
		case UV_EDESTADDRREQ:
		{
			static const std::error_code e = make_error_code(std::errc::destination_address_required);
			return e;
		}
		case UV_EEXIST:
		{
			static const std::error_code e = make_error_code(std::errc::file_exists);
			return e;
		}
		case UV_EFAULT:
		{
			static const std::error_code e = make_error_code(std::errc::bad_address);
			return e;
		}
		case UV_EFBIG:
		{
			static const std::error_code e = make_error_code(std::errc::file_too_large);
			return e;
		}
		case UV_EHOSTDOWN:
		{
			static const std::error_code e = make_error_code(praktor::errc::host_is_down);
			return e;
		}
		case UV_EHOSTUNREACH:
		{
			static const std::error_code e = make_error_code(std::errc::host_unreachable);
			return e;
		}
		case UV_EINTR:
		{
			static const std::error_code e = make_error_code(std::errc::interrupted);
			return e;
		}
		case UV_EINVAL:
		{
			static const std::error_code e = make_error_code(std::errc::invalid_argument);
			return e;
		}
		case UV_EIO:
		{
			static const std::error_code e = make_error_code(std::errc::io_error);
			return e;
		}
		case UV_EISCONN:
		{
			static const std::error_code e = make_error_code(std::errc::already_connected);
			return e;
		}
		case UV_EISDIR:
		{
			static const std::error_code e = make_error_code(std::errc::is_a_directory);
			return e;
		}
		case UV_ELOOP:
		{
			static const std::error_code e = make_error_code(std::errc::too_many_symbolic_link_levels);
			return e;
		}
		case UV_EMFILE:
		{
			static const std::error_code e = make_error_code(std::errc::too_many_files_open);
			return e;
		}
		case UV_EMLINK:
		{
			static const std::error_code e = make_error_code(std::errc::too_many_links);
			return e;
		}
		case UV_EMSGSIZE:
		{
			static const std::error_code e = make_error_code(std::errc::message_size);
			return e;
		}
		case UV_ENAMETOOLONG:
		{
			static const std::error_code e = make_error_code(std::errc::filename_too_long);
			return e;
		}
		case UV_ENETDOWN:
		{
			static const std::error_code e = make_error_code(std::errc::network_down);
			return e;
		}
		case UV_ENETUNREACH:
		{
			static const std::error_code e = make_error_code(std::errc::network_unreachable);
			return e;
		}
		case UV_ENFILE:
		{
			static const std::error_code e = make_error_code(std::errc::too_many_files_open_in_system);
			return e;
		}
		case UV_ENOBUFS:
		{
			static const std::error_code e = make_error_code(std::errc::no_buffer_space);
			return e;
		}
		case UV_ENODEV:
		{
			static const std::error_code e = make_error_code(std::errc::no_such_device);
			return e;
		}
		case UV_ENOENT:
		{
			static const std::error_code e = make_error_code(std::errc::no_such_file_or_directory);
			return e;
		}
		case UV_ENOMEM:
		{
			static const std::error_code e = make_error_code(std::errc::not_enough_memory);
			return e;
		}
		case UV_ENONET:
		{
			static const std::error_code e = make_error_code(praktor::errc::no_network_connection);
			return e;
		}
		case UV_ENOPROTOOPT:
		{
			static const std::error_code e = make_error_code(std::errc::no_protocol_option);
			return e;
		}
		case UV_ENOSPC:
		{
			static const std::error_code e = make_error_code(std::errc::no_space_on_device);
			return e;
		}
		case UV_ENOSYS:
		{
			static const std::error_code e = make_error_code(std::errc::function_not_supported);
			return e;
		}
		case UV_ENOTCONN:
		{
			static const std::error_code e = make_error_code(std::errc::not_connected);
			return e;
		}
		case UV_ENOTDIR:
		{
			static const std::error_code e = make_error_code(std::errc::not_a_directory);
			return e;
		}
		case UV_ENOTEMPTY:
		{
			static const std::error_code e = make_error_code(std::errc::directory_not_empty);
			return e;
		}
		case UV_ENOTSOCK:
		{
			static const std::error_code e = make_error_code(std::errc::not_a_socket);
			return e;
		}
		case UV_ENOTSUP:
		{
			static const std::error_code e = make_error_code(std::errc::not_supported);
			return e;
		}
		case UV_ENOTTY:
		{
			static const std::error_code e = make_error_code(std::errc::inappropriate_io_control_operation);
			return e;
		}
		case UV_ENXIO:
		{
			static const std::error_code e = make_error_code(std::errc::no_such_device_or_address);
			return e;
		}
		case UV_EOF:
		{
			static const std::error_code e = make_error_code(praktor::errc::end_of_file);
			return e;
		}
		case UV_EPERM:
		{
			static const std::error_code e = make_error_code(std::errc::operation_not_permitted);
			return e;
		}
		case UV_EPIPE:
		{
			static const std::error_code e = make_error_code(std::errc::broken_pipe);
			return e;
		}
		case UV_EPROTO:
		{
			static const std::error_code e = make_error_code(std::errc::protocol_error);
			return e;
		}
		case UV_EPROTONOSUPPORT:
		{
			static const std::error_code e = make_error_code(std::errc::protocol_not_supported);
			return e;
		}
		case UV_EPROTOTYPE:
		{
			static const std::error_code e = make_error_code(std::errc::wrong_protocol_type);
			return e;
		}
		case UV_ERANGE:
		{
			static const std::error_code e = make_error_code(std::errc::result_out_of_range);
			return e;
		}
		case UV_EREMOTEIO:
		{
			static const std::error_code e = make_error_code(praktor::errc::remote_io_error);
			return e;
		}
		case UV_EROFS:
		{
			static const std::error_code e = make_error_code(std::errc::read_only_file_system);
			return e;
		}
		case UV_ESHUTDOWN:
		{
			static const std::error_code e = make_error_code(praktor::errc::transport_endpoint_shutdown);
			return e;
		}
		case UV_ESPIPE:
		{
			static const std::error_code e = make_error_code(std::errc::invalid_seek);
			return e;
		}
		case UV_ESRCH:
		{
			static const std::error_code e = make_error_code(std::errc::no_such_process);
			return e;
		}
		case UV_ETIMEDOUT:
		{
			static const std::error_code e = make_error_code(std::errc::timed_out);
			return e;
		}
		case UV_ETXTBSY:
		{
			static const std::error_code e = make_error_code(std::errc::text_file_busy);
			return e;
		}
		case UV_EXDEV:
		{
			static const std::error_code e = make_error_code(std::errc::cross_device_link);
			return e;
		}
		case UV_UNKNOWN:
		default:
		{
			static const std::error_code e = make_error_code(praktor::errc::unknown_error);
			return e;
		}
	}
}

#endif    // PRAKTOR_UV_ERROR_H

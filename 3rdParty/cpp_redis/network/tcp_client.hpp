// MIT License
//
// Copyright (c) 2016-2017 Simon Ninon <simon.ninon@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <cpp_redis/network/tcp_client_iface.hpp>
#include <cpp_redis/redis_error.hpp>


#include "../../Session.h"

namespace cpp_redis
{

namespace network
{

class tcp_client : public tcp_client_iface
{
public:
	//! ctor & dtor
	tcp_client( void ) = default;
	~tcp_client( void ) = default;

public:
	//! start & stop the tcp client
	void connect( const std::string& addr, std::uint32_t port )
	{
		m_client.Connect( addr, port );
	}

	void disconnect( bool wait_for_removal = false )
	{
		m_client.Disconnect( wait_for_removal );
	}

	//! returns whether the client is currently connected or not
	bool is_connected( void ) const
	{
		return m_client.IsConnected();
	}

public:
	//! async read & write operations
	void async_read( read_request& request )
	{
		auto callback = std::move( request.async_read_callback );

		read_result result;
		try
		{
			request.size = m_client.RecvBuffer( result.buffer, request.size );
			result.success = true;
		}
		catch ( ... )
		{
			result.success = false;
		}

		if( !callback )
			return;

		callback( result );
	}

	void async_write( write_request& request )
	{
		auto callback = std::move( request.async_write_callback );
		write_result converted_result;

		converted_result.size = m_client.SendBuffer( request.buffer, request.buffer.size() );
		
		if( !callback )
			return;

		if( converted_result.size > 0 )
			converted_result.success = true;

		callback( converted_result );
	}

public:
	//! set on disconnection handler
	void set_on_disconnection_handler( const disconnection_handler_t& disconnection_handler )
	{
		m_client.SetOnDisconnectionHandler( disconnection_handler );
	}

private:
	//! tcp client for redis connection
	Session m_client;
};

} //! network

} //! cpp_redis

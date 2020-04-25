//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_CONNECTION_HPP
#define HTTP_SERVER3_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"

namespace http 
{
	namespace server3 
	{
		/// Represents a single connection from a client.
		class connection
			: public boost::enable_shared_from_this<connection>
			, private boost::noncopyable
		{
		public:
			/// Construct a connection with the given io_service.
			explicit connection(boost::asio::io_service& io_service, boost::shared_ptr<request_handler> handler);

			/// Get the socket associated with the connection.
			boost::asio::ip::tcp::socket& socket();
			/// Start the first asynchronous operation for the connection.
			void start();
            void stop();
			void async_write(reply& rep);

			request& get_request()
			{
				return request_;
			}

			reply& get_reply()
			{
				return reply_;
			}

			boost::shared_ptr<request_handler>& get_request_handler()
			{
				return request_handler_;
			}
		private:
			/// Handle completion of a read operation.
			void handle_read(const boost::system::error_code& e, std::size_t bytes_transferred);

			void handle_read_body(const boost::system::error_code& e, std::size_t bytes_transferred);

			/// Handle completion of a write operation.
			void handle_write(const boost::system::error_code& e);

			/// Strand to ensure the connection's handlers are not called concurrently.
			boost::asio::io_service::strand strand_;

			/// Socket for the connection.
			boost::asio::ip::tcp::socket socket_;

			/// The handler used to process the incoming request.
			boost::shared_ptr<request_handler> request_handler_;

			/// Buffer for incoming data.
			boost::array<char, 8192> buffer_;

			/// The incoming request.
			request request_;

			/// The parser for the incoming request.
			request_parser request_parser_;

			/// The reply to be sent back to the client.
			reply reply_;

			UINT post_content_size;
            enum HEADER_STATE
            {
                good,
                bad,
                indeterminate
            };
            HEADER_STATE m_headerState;
		};

		typedef boost::shared_ptr<connection> connection_ptr;

	} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_CONNECTION_HPP

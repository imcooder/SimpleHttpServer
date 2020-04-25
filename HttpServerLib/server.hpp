//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_SERVER_HPP
#define HTTP_SERVER3_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "connection.hpp"
#include "request_handler.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace http 
{
	namespace server3 
	{
		/// The top-level class of the HTTP server.
		class server
			: private boost::noncopyable
			, public boost::enable_shared_from_this<server>
		{
		public:
			/// Construct the server to listen on the specified TCP address and port, and
			/// serve up files from the given directory.
			explicit server(boost::shared_ptr<request_handler>request_h_, std::size_t thread_pool_size, boost::mutex &dealtimer_mu);
			void Init(const std::string& address, int port);
			virtual~server();
			/// Run the server's io_service loop.
			void run();

			void stop();

			bool is_open();

		private:
			/// Initiate an asynchronous accept operation.
			void start_accept();

			/// Handle completion of an asynchronous accept operation.
			void handle_accept(const boost::system::error_code& e);

			/// Handle a request to stop the server.
			void handle_stop();

			void deal_time() ;

			void ThreadProc();
			/// The number of threads that will call io_service::run().
			std::size_t thread_pool_size_;

			/// The io_service used to perform asynchronous operations.
			boost::asio::io_service io_service_;

			/// The signal_set is used to register for process termination notifications.
			boost::asio::signal_set signals_;

			/// Acceptor used to listen for incoming connections.
			boost::asio::ip::tcp::acceptor acceptor_;

			/// The next connection to be accepted.
			connection_ptr new_connection_;

			/// The handler for all incoming requests.
			boost::shared_ptr<request_handler> request_handler_;

			///Deal timer mutex for io_service.run working.
			boost::mutex &m_dealtimer_mu ;

		};

	} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_SERVER_HPP

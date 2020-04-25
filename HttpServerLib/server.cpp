//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "stdafx.h"
#include "server.hpp"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "request_handler.hpp"
#include "httplib/bddebug.h"
namespace http 
{
	namespace server3 
	{
		server::server(boost::shared_ptr<request_handler>request_h_, std::size_t thread_pool_size, boost::mutex &dealtimer_mu)
			: thread_pool_size_(thread_pool_size),
			signals_(io_service_),
			acceptor_(io_service_),
			new_connection_(),
			request_handler_(request_h_),
			m_dealtimer_mu(dealtimer_mu)
		{
			XLOG_FUNCTION;

		}
		server::~server()
		{
			XLOG_FUNCTION;
		}
		void server::Init(const std::string& address, int port)
		{
			XLOG_FUNCTION;
			// Register to handle the signals that indicate when the server should exit.
			// It is safe to register for the same signal multiple times in a program,
			// provided all registration for the specified signal is made through Asio.
			signals_.add(SIGINT);
			signals_.add(SIGTERM);
#if defined(SIGQUIT)
			signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
			signals_.async_wait(boost::bind(&server::handle_stop, shared_from_this()));

			boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address().from_string(address), port);
			acceptor_.open(boost::asio::ip::tcp::endpoint::protocol_type::v4());

			boost::system::error_code ec ;
			acceptor_.bind(endpoint,ec);
			if(ec)
			{
				acceptor_.close() ;
			}
			else
			{
				acceptor_.listen();
				start_accept();
			}
		}
		void server::run()
		{
			XLOG_FUNCTION;
			//add timer (timeout == 50)
			boost::asio::deadline_timer timer_(io_service_) ;
			// Create a pool of threads to run all of the io_services.
			std::vector<boost::shared_ptr<boost::thread> > threads;
			for (std::size_t i = 0; i < thread_pool_size_; ++i)
			{
				boost::shared_ptr<boost::thread> thread(new boost::thread(boost::bind(&server::ThreadProc, shared_from_this())));
				threads.push_back(thread);
			}
			//通过延时进行同步
			//boost::this_thread::sleep(boost::posix_time::millisec(50)) ;
			//m_dealtimer_mu.unlock() ;

			//通过定时器进行同步
			timer_.expires_from_now(boost::posix_time::millisec(50)) ;
			timer_.async_wait(boost::bind(&server::deal_time, shared_from_this())) ;

			// Wait for all threads in the pool to exit.
			for (std::size_t i = 0; i < threads.size(); ++i)
			{
				threads[i]->join();
			}
		}
		void server::ThreadProc()
		{
			//XLOG_FUNCTION;
			boost::system::error_code ec;
			io_service_.run(ec);
			BD_VERIFY(!ec, return);
		}
		void server::stop()
		{
			signals_.cancel();
		}

		void server::start_accept()
		{
			new_connection_.reset(new(std::nothrow) connection(io_service_, request_handler_));
			BD_VERIFY(new_connection_, return);
			acceptor_.async_accept(new_connection_->socket(), boost::bind(&server::handle_accept, shared_from_this(), boost::asio::placeholders::error));
		}
		void server::handle_accept(const boost::system::error_code& e)
		{
			if (!e)
			{
				new_connection_->start();
			}
			start_accept();
		}
		void server::handle_stop()
		{
			io_service_.stop();
		}
		bool server::is_open()
		{
			return acceptor_.is_open() ;
		}

		void server::deal_time()
		{
			m_dealtimer_mu.unlock() ;
		}

	} // namespace server3
} // namespace http

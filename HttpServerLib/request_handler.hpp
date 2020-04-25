//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_REQUEST_HANDLER_HPP
#define HTTP_SERVER3_REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
namespace http
{
	namespace server3
	{
		struct reply;
		struct request;
		class connection;
		typedef boost::function<bool (boost::shared_ptr<connection>)> func_handle_request;
		/// The common handler for all incoming requests.
		class request_handler
		{
		public:
			/// Handle a request and produce a reply.
			virtual void handle_request(boost::shared_ptr<connection> pConnection);
			void Hook_HandleRequest(func_handle_request func)
			{
				m_func = func;
				return;
			}
		protected:
			func_handle_request m_func;
			void ParserRequest(boost::shared_ptr<connection>);
			std::string UrlDecode( const std::string& in );
		};

	} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_HANDLER_HPP

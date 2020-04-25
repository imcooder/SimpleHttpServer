//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "stdafx.h"
#include "request_handler.hpp"
#include "HttpLib/bddebug.h"
#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"
#include "connection.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/xpressive/xpressive_dynamic.hpp>
namespace http 
{
	namespace server3 
	{
		void request_handler::handle_request(boost::shared_ptr<connection> pConnection)
		{
			//WEB_DBG(L"request_handler::handle_request");
			BD_VERIFY(pConnection, return);
			ParserRequest(pConnection);
			if(m_func)
			{
				m_func(pConnection);
			}
		}

		std::string request_handler::UrlDecode(const std::string& in)
		{			
			std::string out ;
			out.reserve(in.size());
			for (std::size_t i = 0; i < in.size(); ++i)
			{
				if (in[i] == '%')
				{
					if (i + 3 <= in.size())
					{
						int value = 0;
						std::istringstream is(in.substr(i + 1, 2));
						if (is >> std::hex >> value)
						{
							out += static_cast<char>(value);
							i += 2;
						}
						else
						{
							return "";
						}
					}
					else
					{
						return "";
					}
				}
				else if (in[i] == '+')
				{
					out += ' ';
				}
				else
				{
					out += in[i];
				}
			}
			return out;		
		}

		void request_handler::ParserRequest(boost::shared_ptr<connection>pConnection)
		{
			BD_VERIFY(pConnection, return);
			request& req = pConnection->get_request();
			reply& rep = pConnection->get_reply();
			std::string request_path = req.uri ;
			std::size_t first_question_pos = request_path.find_first_of("?");
			if (first_question_pos != string::npos && first_question_pos > 0)
			{
				req.path = request_path.substr(0, first_question_pos);
			}
			else
			{
				req.path = request_path;
			}
			
			std::string strParams = request_path.substr(first_question_pos + 1);
			strParams += "&";
			boost::xpressive::sregex expr = boost::xpressive::sregex::compile("(.*?)=(.*?)[&$]") ;
			boost::xpressive::sregex_iterator pos(strParams.begin(),strParams.end(),expr) ;
			boost::xpressive::sregex_iterator end ;
			while(pos != end)
			{
				std::string key = UrlDecode( (*pos)[1] );
				std::string value = UrlDecode( (*pos)[2] );
				req.mapArgs[key] = value;		
				++pos;
			}
			return;
		}

	} // namespace server3
} // namespace http

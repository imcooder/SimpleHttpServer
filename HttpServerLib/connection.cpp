//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "stdafx.h"
#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "request_handler.hpp"

namespace http
{
    namespace server3
    {

        connection::connection(boost::asio::io_service& io_service,
                               boost::shared_ptr<request_handler> handler)
                               : strand_(io_service)
                               , socket_(io_service)
                               , request_handler_(handler)
                               , post_content_size(0)
                               , m_headerState(indeterminate)
        {
        }

        boost::asio::ip::tcp::socket& connection::socket()
        {
            return socket_;
        }

        void connection::start()
        {
            buffer_.fill('\0');
            socket_.async_read_some(boost::asio::buffer(buffer_),
                                    strand_.wrap(
                                    boost::bind(&connection::handle_read,
                                    shared_from_this(),
                                    boost::asio::placeholders::error,
                                    boost::asio::placeholders::bytes_transferred)));
        }
        void connection::stop()
        {
            socket_.close();
        }
        void connection::async_write(reply& rep)
        {
            // boost::asio::write(socket_, rep.to_buffers());
            boost::asio::async_write(socket_, rep.to_buffers(),
                                     strand_.wrap(
                                     boost::bind(&connection::handle_write, 
                                     shared_from_this(),
                                     boost::asio::placeholders::error)));
        }

        void connection::handle_read(const boost::system::error_code& e,
                                     std::size_t bytes_transferred)
        {
            if (!e)
            {
                if (m_headerState == indeterminate)
                {
                    boost::tribool result;
                    boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
                        request_, buffer_.data(), buffer_.data() + bytes_transferred);
                    if (result)
                    {
                        m_headerState = good;
                    }
                    else if (result == boost::logic::indeterminate)
                    {
                        m_headerState = indeterminate;
                    }
                    else
                    {
                        m_headerState = bad;
                    }
                }
                if (m_headerState == good)
                {
                    if (stricmp(request_.method.c_str(), "POST") == 0)
                    {                        
                        post_content_size = boost::lexical_cast<UINT>(request_.GetHeader("content-length", "0"));
                        if (post_content_size > 0)
                        {
                            UINT dPos =
                                boost::lexical_cast<std::string>(buffer_.data()).find(
                                "\r\n\r\n");
                            if (bytes_transferred - (dPos + 4) > 0)
                            {
                                request_.post_content.append(buffer_.data() + dPos + 4, bytes_transferred - (dPos + 4));
                            }
                            else
                            {
                                request_.post_content.clear();
                            }
                        }
                        if (request_.post_content.size() < post_content_size)
                        {
                            buffer_.fill('\0');
                            socket_.async_read_some(boost::asio::buffer(buffer_),
                                strand_.wrap(
                                boost::bind(&connection::handle_read_body,
                                shared_from_this(),
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred)));
                        }
                        else
                        {
                            //调用处理函数，待定
                            request_handler_->handle_request(shared_from_this());
                        }
                    }
                    else
                    {
                        request_handler_->handle_request(shared_from_this());
                    }
                }
                else if (m_headerState == bad)
                {
                    reply_ = reply::stock_reply(reply::bad_request);
                    boost::asio::async_write(socket_, reply_.to_buffers(), strand_.wrap(
                        boost::bind(&connection::handle_write, 
                        shared_from_this(), 
                        boost::asio::placeholders::error)));
                }
                else
                {
                    socket_.async_read_some(boost::asio::buffer(buffer_), strand_.wrap(
                        boost::bind(&connection::handle_read, 
                        shared_from_this(),
                        boost::asio::placeholders::error, 
                        boost::asio::placeholders::bytes_transferred)));
                }
            }

            // If an error occurs then no new asynchronous operations are started. This
            // means that all shared_ptr references to the connection object will
            // disappear and the object will be destroyed automatically after this
            // handler returns. The connection class's destructor closes the socket.
        }

        void connection::handle_write(const boost::system::error_code& e)
        {
            if (!e)
            {
                // Initiate graceful connection closure.
                boost::system::error_code ignoredEC;
                socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignoredEC);
            }

            // No new asynchronous operations are started. This means that all shared_ptr
            // references to the connection object will disappear and the object will be
            // destroyed automatically after this handler returns. The connection class's
            // destructor closes the socket.
        }

        void connection::handle_read_body(const boost::system::error_code& e, 
                                          std::size_t bytes_transferred)
        {
            if (!e)
            {
                if (bytes_transferred)
                {
                    request_.post_content.append(buffer_.data(), bytes_transferred);
                }
                if (request_.post_content.size() < post_content_size)
                {
                    buffer_.fill('\0');
                    socket_.async_read_some(boost::asio::buffer(buffer_),
                        strand_.wrap(boost::bind(&connection::handle_read_body,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
                }
                else
                {
                    //调用处理函数
                    request_handler_->handle_request(shared_from_this());
                }
            }
        }
    } // namespace server3
} // namespace http

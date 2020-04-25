#include "pch.h"
#include "SocketClient.h"
#include <boost/thread.hpp>
//#include <stdlib>
#include <strstream>
using namespace std;
SocketClient::SocketClient( boost::asio::io_service& io_service, const std::string& server, const std::string& path, int port ) : resolver_(io_service),
	socket_(io_service),
	t(io_service, boost::posix_time::millisec(100))
{
	// Form the request. We specify the "Connection: close" header so that the
	// server will close the socket after transmitting the response. This will
	// allow us to treat all data up until the EOF as the content.
	std::ostream request_stream(&request_);
	request_stream << "GET " << path << " HTTP/1.0\r\n";
	request_stream << "Host: " << server << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Connection: close\r\n\r\n";
	m_strServer = server;
	m_nPort = port;
}

void SocketClient::handle_resolve( const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator )
{
	if (!err)
	{
		// Attempt a connection to each endpoint in the list until we
		// successfully establish a connection.
		try 
		{
			boost::asio::async_connect(socket_, endpoint_iterator, boost::bind(&SocketClient::handle_connect, shared_from_this(), boost::asio::placeholders::error));
		}
		catch (boost::thread_interrupted&)
		{
			stringstream ss;
			ss << "Thread  " << GetCurrentThreadId() << "interrupted";
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
		catch (std::exception& e)
		{
			stringstream ss;
			ss << "Exception caught from thread " << GetCurrentThreadId() << " " << e.what();
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
		catch (...) 
		{
			stringstream ss;
			ss << "Unknown exception caught from thread " << GetCurrentThreadId();
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
	}
	else
	{
		std::cout << "Error: " << err.message() << "\n";
	}
}

void SocketClient::handle_connect( const boost::system::error_code& err )
{
	if (!err)
	{
		// The connection was successful. Send the request.
		try 
		{
			boost::asio::async_write(socket_, request_, boost::bind(&SocketClient::handle_write_request, shared_from_this(), boost::asio::placeholders::error));
		}
		catch (boost::thread_interrupted&)
		{
			stringstream ss;
			ss << "Thread  " << GetCurrentThreadId() << "interrupted";
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
		catch (std::exception& e)
		{
			stringstream ss;
			ss << "Exception caught from thread " << GetCurrentThreadId() << " " << e.what();
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
		catch (...) 
		{
			stringstream ss;
			ss << "Unknown exception caught from thread " << GetCurrentThreadId();
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
	}
	else
	{
		std::cout << "Error: " << err.message() << "\n";
	}
}

void SocketClient::handle_write_request( const boost::system::error_code& err )
{
	if (!err)
	{
		// Read the response status line. The response_ streambuf will
		// automatically grow to accommodate the entire line. The growth may be
		// limited by passing a maximum size to the streambuf constructor.		
		try 
		{
			boost::asio::async_read_until(socket_, response_, "\r\n", boost::bind(&SocketClient::handle_read_status_line, shared_from_this(), boost::asio::placeholders::error));
		}
		catch (boost::thread_interrupted&)
		{
			stringstream ss;
			ss << "Thread  " << GetCurrentThreadId() << "interrupted";
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
		catch (std::exception& e)
		{
			stringstream ss;
			ss << "Exception caught from thread " << GetCurrentThreadId() << " " << e.what();
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
		catch (...) 
		{
			stringstream ss;
			ss << "Unknown exception caught from thread " << GetCurrentThreadId();
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
	}
	else
	{
		std::cout << "Error: " << err.message() << "\n";
	}
}

void SocketClient::handle_read_status_line( const boost::system::error_code& err )
{
	if (!err)
	{
		// Check that response is OK.
		std::istream response_stream(&response_);
		std::string http_version;
		response_stream >> http_version;
		unsigned int status_code;
		response_stream >> status_code;
		std::string status_message;
		std::getline(response_stream, status_message);
		if (!response_stream || http_version.substr(0, 5) != "HTTP/")
		{
			std::cout << "Invalid response\n";
			return;
		}
		if (status_code != 200)
		{
			std::cout << "Response returned with status code ";
			std::cout << status_code << "\n";
			return;
		}
		// Read the response headers, which are terminated by a blank line.
		try 
		{
			boost::asio::async_read_until(socket_, response_, "\r\n\r\n", boost::bind(&SocketClient::handle_read_headers, shared_from_this(),  boost::asio::placeholders::error));
		}
		catch (boost::thread_interrupted&)
		{
			stringstream ss;
			ss << "Thread  " << GetCurrentThreadId() << "interrupted";
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
		catch (std::exception& e)
		{
			stringstream ss;
			ss << "Exception caught from thread " << GetCurrentThreadId() << " " << e.what();
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
		catch (...) 
		{
			stringstream ss;
			ss << "Unknown exception caught from thread " << GetCurrentThreadId();
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
	}
	else
	{
		std::cout << "Error: " << err << "\n";
	}
}

void SocketClient::handle_read_headers( const boost::system::error_code& err )
{
	if (!err)
	{
		// Process the response headers.
		std::istream response_stream(&response_);
		std::string header;
		while (std::getline(response_stream, header) && header != "\r")
		{
			//std::cout << header << "\n";
		}
		//std::cout << "\n";

		// Write whatever content we already have to output.
		// 		if (response_.size() > 0)
		// 			std::cout << &response_;

		// Start reading remaining data until EOF.
		try 
		{
			boost::asio::async_read(socket_, response_, boost::asio::transfer_at_least(1), boost::bind(&SocketClient::handle_read_content, shared_from_this(), boost::asio::placeholders::error));
		}
		catch (boost::thread_interrupted&)
		{
			stringstream ss;
			ss << "Thread  " << GetCurrentThreadId() << "interrupted";
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
		catch (std::exception& e)
		{
			stringstream ss;
			ss << "Exception caught from thread " << GetCurrentThreadId() << " " << e.what();
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
		catch (...) 
		{
			stringstream ss;
			ss << "Unknown exception caught from thread " << GetCurrentThreadId();
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
	}
	else
	{
		std::cout << "Error: " << err << "\n";
	}
}

void SocketClient::handle_read_content( const boost::system::error_code& err )
{
	if (!err)
	{
		// Write all of the data that has been read so far.
		//		std::cout << &response_;

		// Continue reading remaining data until EOF.
		try 
		{
			boost::asio::async_read(socket_, response_, boost::asio::transfer_at_least(1), boost::bind(&SocketClient::handle_read_content, shared_from_this(), boost::asio::placeholders::error));
		}
		catch (boost::thread_interrupted&)
		{
			stringstream ss;
			ss << "Thread  " << GetCurrentThreadId() << "interrupted";
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
		catch (std::exception& e)
		{
			stringstream ss;
			ss << "Exception caught from thread " << GetCurrentThreadId() << " " << e.what();
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}
		catch (...) 
		{
			stringstream ss;
			ss << "Unknown exception caught from thread " << GetCurrentThreadId();
			string strValue = ss.str();
			XLogError(L"%S", strValue.c_str());
		}		
	}
	else if (err != boost::asio::error::eof)
	{
		std::cout << "Error: " << err << "\n";
	}
}

void SocketClient::timeout()
{
	socket_.close() ;
}

void SocketClient::Init()
{	
	char szPort[64] = {0};
	_itoa_s(m_nPort, szPort, 10);
	tcp::resolver::query query(m_strServer, szPort);
	resolver_.async_resolve(query,boost::bind(&SocketClient::handle_resolve, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::iterator));
	t.async_wait(boost::bind(&SocketClient::timeout, shared_from_this()));
}

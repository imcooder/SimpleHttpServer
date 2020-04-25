#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
using boost::asio::ip::tcp;

class SocketClient : public boost::enable_shared_from_this<SocketClient>
{
public:
	SocketClient(boost::asio::io_service& io_service, const std::string& server, const std::string& path, int nPort =5808);
public:
	void Init();
private:
	void handle_resolve(const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator);
	void handle_connect(const boost::system::error_code& err);
	void handle_write_request(const boost::system::error_code& err);
	void handle_read_status_line(const boost::system::error_code& err);
	void handle_read_headers(const boost::system::error_code& err);
	void handle_read_content(const boost::system::error_code& err);
	void timeout() ;
public:
	boost::asio::streambuf response_;
private:
	tcp::resolver resolver_;
	tcp::socket socket_;
	boost::asio::streambuf request_;
	boost::asio::deadline_timer t ;
	std::string m_strServer;
	int m_nPort;
};
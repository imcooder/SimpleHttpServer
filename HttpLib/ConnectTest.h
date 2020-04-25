#pragma once
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#define PORT_NUM 5

class ConnectTest : public boost::enable_shared_from_this<ConnectTest>
{
public:
	ConnectTest(const std::string& host);
	virtual ~ConnectTest(void){};
public:
	void TestPort(int port, bool &ret);
	int SelectPort(const int* ports, size_t uCount);
	//every try takes 100ms
	int SelectPort(const int* ports, size_t uCount, int try_time);
private:
	std::string m_host ;
	int streambuf2string(boost::asio::streambuf &buf,std::string &str) ;
};


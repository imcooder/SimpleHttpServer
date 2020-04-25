#include "pch.h"
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include "SocketClient.h"
#include "ConnectTest.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "bddebug.h"
ConnectTest::ConnectTest( const std::string& host ) :m_host(host)
{

}

void ConnectTest::TestPort(int nPort, bool &ret )
{
	XLogDebug(L"TestPort::%d", nPort);
	boost::asio::io_service io_service;
	boost::shared_ptr<SocketClient> socket_ = boost::make_shared<SocketClient>(io_service, m_host, "/test.php", nPort);
	socket_->Init();
	boost::system::error_code ignored_ec;
	io_service.run(ignored_ec);
	if (ignored_ec)
	{
		ret = false;
		BD_VERIFY(0, return);
		return;
	}
	std::string str("") ;
	streambuf2string(socket_->response_, str);
	if(str != "test OK ffff")
	{
		ret = false;
		return;
	}
	ret = true;
}

int ConnectTest::SelectPort(const int* ports, size_t uCount)
{
	BD_VERIFY(ports && uCount> 0, return -1);
	bool *ret = new(std::nothrow) bool[uCount];
	BD_VERIFY(ret, return -1);	
	ON_LEAVE_1(delete[]ret, bool*, ret);
	boost::thread_group threads;
	for(int i = 0; i < uCount; i ++)
	{
		ret[i] = false;
		threads.create_thread(boost::bind(&ConnectTest::TestPort, shared_from_this(), ports[i], boost::ref(ret[i])));
	}
	threads.join_all();
	for(int i = 0; i < uCount; i ++)
	{
		if(ret[i])
		{
			return ports[i];
		}
	}
	return -1;
}

int ConnectTest::SelectPort(const int* ports, size_t uCount, int try_time )
{
	for(int i = 0; i < try_time; i ++)
	{
		int port = SelectPort(ports, uCount) ;
		if(port!= -1)
		{
			return port ;
		}
	}
	return -1 ;
}


int ConnectTest::streambuf2string(boost::asio::streambuf &buf,std::string &str)
{
	//获取请求结果
	int n = buf.size() ;
	if(n <= 0)
	{
		return -1 ;
	}
	str.resize(n) ;
	std::istream s(&buf);
	s.read(&str.front(), n);
	return n ;
}

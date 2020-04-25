#include "stdafx.h"
#include "HttpServiceImp.h"
#include "HttpLib/ConnectTest.h"
#include "request_handler.hpp"
#include "HttpLib/bddebug.h"
#include <strsafe.h>
#include <atlstr.h>
#include <UserEnv.h>
#define HTTP_DBG(...)  XLogDebug( XTAG(L"http"), __VA_ARGS__);
#define HTTP_LOG(...)  XLogInfo( XTAG(L"http"), __VA_ARGS__);
#define HTTP_ERR(...)  XLogError( XTAG(L"http"), __VA_ARGS__);
#define HTTP_WAR(...)  XLogWarning( XTAG(L"http"), __VA_ARGS__);
#define HTTP_CRT(...)  XLogCritical( XTAG(L"http"), __VA_ARGS__);
namespace http
{
	CHttpServiceImp::CHttpServiceImp(size_t nReqTreadCount, size_t nThreadsPoolCount)
		: m_uReqThreadCount(nReqTreadCount)
		, m_uRepThreadCount(nThreadsPoolCount)
	{
		HTTP_DBG(L"CHttpServiceImp");
		m_nPort = 0;
	}

	CHttpServiceImp::~CHttpServiceImp()
	{
		HTTP_DBG(L"~CHttpService");
		{
			CSWMRG::CAutoWriteLock _(m_rwMap);
			m_mapCallback.clear();
		}
	}
	int CHttpServiceImp::StartServer(const int ports[], size_t count)
	{
		HTTP_DBG(L"CHttpService::StartServer");
		StartThreads();
		const char*host = "0.0.0.0";
		//////////////////////////////////////////////////////////////////////////
		HTTP_DBG(L"HTTP服务程序开始------------------------------------------------"); 
		try
		{
			//create ContentFactory takes about 100ms
			//create RequestHandlerImpl takes about 0.1ms
			boost::shared_ptr<http::server3::request_handler> request_handler = boost::make_shared<http::server3::request_handler>();
			request_handler->Hook_HandleRequest(boost::bind(&CHttpServiceImp::HandleRequestCallback, shared_from_this(), _1));
			for(int i = 0; i < count; i ++)
			{
				WORD port = ports[i];
				HTTP_DBG(L"test port:%d", port);	
				boost::mutex dealtimer_mu ;
				m_pServer = boost::make_shared<http::server3::server>(request_handler, m_uReqThreadCount, dealtimer_mu);
				BD_VERIFY(m_pServer, continue);
				//检测server指定端口是否启动
				m_pServer->Init(host, port);
				if(m_pServer->is_open() == false)
				{
					continue;
				}
				{
					HTTP_DBG(L"服务线程开始启动...");
					dealtimer_mu.lock() ;
					m_server_thread = boost::make_shared<boost::thread>(boost::bind(&CHttpServiceImp::RunServer_Internal, shared_from_this()));
					boost::mutex::scoped_lock lock(dealtimer_mu);
				}
				HTTP_DBG(L"端口测试开始...");
				int n = 0 ;
				while(n < 3)
				{
					if(ServerTest(port))
					{
						m_nPort = port;	
						break;
					}
					boost::this_thread::sleep(boost::posix_time::millisec(50));
					n++ ;
				}
				if(n == 3)
				{                
					HTTP_WAR(L"端口测试失败：port %d", port);
					StopServer();
					continue ;
				}
				HTTP_DBG(L"端口测试成功：port %d", port);
				break;
			}
		}
		catch(std::exception& )
		{
			HTTP_ERR(L"启动server 遇到异常！");
			return 0;
		}
		return m_nPort;
	}

	void CHttpServiceImp::RunServer_Internal()
	{
		m_pServer->run();
	}

	bool CHttpServiceImp::ServerTest(int nPort)
	{
		bool ret(false) ;
		boost::shared_ptr<ConnectTest> test(new(std::nothrow) ConnectTest("127.0.0.1"));
		BD_VERIFY(test, return false);
		test->TestPort(nPort, ret ) ;
		return ret;
	}

	bool CHttpServiceImp::StopServer()
	{
		HTTP_DBG(L"CHttpService::StopServer");
		if(m_pServer)
		{
			m_pServer->stop();
		}
		if(m_server_thread) 
		{
			m_server_thread->join();
		}
		return true;
	}

	bool CHttpServiceImp::HandleRequestCallback(boost::shared_ptr<http::server3::connection>pConnect)
	{
		m_ios.post(boost::bind(&CHttpServiceImp::HandleRequestProc, shared_from_this(), pConnect));
		return true;
	}

	void CHttpServiceImp::ReplyResult(boost::shared_ptr<http::server3::connection> pConnection)
	{
		BD_VERIFY(pConnection, return);
		http::server3::reply& rep = pConnection->get_reply();
		rep.headers.resize(2);
		rep.headers[0].name = "Content-Length";
		rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
		rep.headers[1].name = "Content-Type";
		rep.headers[1].value = http::server3::mime_types::extension_to_type(rep.contentMimeType);
		pConnection->async_write(rep);
		return;
	}

	void CHttpServiceImp::HandleRequestProc(boost::shared_ptr<http::server3::connection>pConnection)
	{
		BD_VERIFY(pConnection, return);
		http::server3::request& req = pConnection->get_request();
		http::server3::reply& rep = pConnection->get_reply();
		rep.status = http::server3::reply::not_found;
		HandleRequest(req, rep);
		ReplyResult(pConnection);
		return;
	}

	void CHttpServiceImp::HandleRequest(http::server3::request&req, http::server3::reply& rep)
	{
		//HTTP_DBG(L"Content ReplyContent::%S", req.uri.c_str());
		std::string url = req.uri.c_str();
		boost::to_lower(url);
		if(req.path.find("/test.php") == 0)
		{
			if (FAILED(HandleTest(req, rep)))
			{
				rep.status = http::server3::reply::forbidden;
			}
			return;
		}
		http_callback c = NULL;
		{
			CSWMRG::CAutoReadLock _(m_rwMap);
			auto it = m_mapCallback.find(req.path);
			BD_VERIFY(it != m_mapCallback.end(), return);
			c = it->second;
		}
		if (c)
		{
			if (FAILED(c(req, rep)))
			{
				rep.status = http::server3::reply::forbidden;
			}
			return;
		}
		return;
	}

	HRESULT CHttpServiceImp::HandleTest(http::server3::request&req, http::server3::reply& rep)
	{
		rep.status = http::server3::reply::ok;
		rep.content = "test OK ffff";
		return S_OK;
	}


	void CHttpServiceImp::OnSuccess()
	{
		HTTP_DBG(L"OnSuccess");
		return;
	}

	HRESULT CHttpServiceImp::StartThreads()
	{
		while (m_threadGroup.size() < m_uRepThreadCount)
		{
			m_threadGroup.create_thread(boost::bind(&CHttpServiceImp::ThreadProc, shared_from_this()));
		}
		return S_OK;
	}
	void CHttpServiceImp::ThreadProc()
	{
		boost::asio::io_service::work work(m_ios);
		boost::system::error_code ec;
		m_ios.run(ec);
		BD_VERIFY(!ec, return);
		return;
	}

	HRESULT CHttpServiceImp::StopThreads()
	{
		m_ios.stop();
		m_threadGroup.interrupt_all();
		m_threadGroup.join_all();
		return S_OK;
	}
	HRESULT CHttpServiceImp::HandleCheckVersion(http::server3::request&, http::server3::reply& rep)
	{
		rep.status = http::server3::reply::ok;	
		rep.content = "OK";
		return S_OK;
	}
	HRESULT CHttpServiceImp::HandleStop(http::server3::request&, http::server3::reply& rep)
	{
		rep.status = http::server3::reply::ok;	
		rep.content = "OK";
		SetTimer(500, boost::bind(&CHttpServiceImp::StopServer, shared_from_this()));
		return S_OK;
	}
	void CHttpServiceImp::SetTimer(int millisec, boost::function<void ()> func)
	{
		boost::shared_ptr<boost::asio::deadline_timer> t = boost::make_shared<boost::asio::deadline_timer>(m_ios, boost::posix_time::millisec(millisec)) ;
		BD_VERIFY(t, return);
		t->async_wait(boost::bind(&CHttpServiceImp::OnTimer, shared_from_this(), func, t, boost::asio::placeholders::error));
	}

	void CHttpServiceImp::OnTimer(boost::function<void ()>func, boost::shared_ptr<boost::asio::deadline_timer> t, const boost::system::error_code &err)
	{
		func();
	}
	HRESULT CHttpServiceImp::Register(const std::string&path, http_callback c )
	{
		BD_VERIFY(c, return E_INVALIDARG);
		CSWMRG::CAutoWriteLock _(m_rwMap);
		m_mapCallback[path] = c;
		return S_OK;
	}
}
#pragma once

#include "httpserverlib/server.hpp"
#include "HttpServerlib/mime_types.hpp"
#include <boost/thread.hpp>
#include <atlstr.h>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "HttpLib/SWMRG.h"
#include <string>
#include <unordered_map>
using namespace std;
#ifndef _HTTP_CALLBACK_DEFINED_
#define _HTTP_CALLBACK_DEFINED_
typedef boost::function<HRESULT (http::server3::request&req, http::server3::reply& rep)> http_callback;
#endif
namespace http
{
	class CHttpServiceImp : public boost::enable_shared_from_this<CHttpServiceImp>
	{
	public:
		CHttpServiceImp(size_t nRwqTreadCount = 2, size_t nThreadsPoolCount = 10);
		virtual ~CHttpServiceImp();
		bool StopServer();
		int StartServer(const int ports[], size_t);	
		HRESULT Register(const std::string&, http_callback);
	protected:	
		bool ServerTest(int nPort);
		void RunServer_Internal();
		void OnSuccess();
		HRESULT StartThreads();
		HRESULT StopThreads();   
		bool HandleRequestCallback(boost::shared_ptr<http::server3::connection>);
		void HandleRequestProc(boost::shared_ptr<http::server3::connection>);
		void ReplyResult( boost::shared_ptr<http::server3::connection>);
		void HandleRequest(http::server3::request&, http::server3::reply& rep);
		HRESULT HandleTest(http::server3::request&, http::server3::reply& rep);
		HRESULT HandleCheckVersion(http::server3::request&, http::server3::reply& rep);
		HRESULT HandleStop(http::server3::request&, http::server3::reply& rep);
		HRESULT HandleElevate(http::server3::request&, http::server3::reply& rep);
		void ThreadProc();
		void SetTimer( int millisec, boost::function<void ()> func );
		void OnTimer(boost::function<void ()>func, boost::shared_ptr<boost::asio::deadline_timer> t,const boost::system::error_code &err);
	protected:
		boost::shared_ptr<boost::thread> m_server_thread ;
		boost::mutex m_server_mu ;
		boost::shared_ptr<http::server3::server> m_pServer;
		int m_nPort;
		boost::thread_group m_threadGroup;
		boost::asio::io_service m_ios;
		CSWMRG m_rwMap;
		unordered_map<string, http_callback> m_mapCallback;
		size_t m_uReqThreadCount;
		size_t m_uRepThreadCount;
	};
};

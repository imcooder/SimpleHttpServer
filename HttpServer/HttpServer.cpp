// HttpServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
#include <iostream>
#include <boost/shared_ptr.hpp>
#include "HttpLib/bddebug.h"
#include "HttpServerLib/HttpServiceImp.h"

boost::asio::io_service io_service;

static const int g_httpports[] = { 5809, 6809, 7809, 8809, 9809, 9809 };
class CSimpleHttpServer
{
public:
    CSimpleHttpServer(int reqThreadNum, int processThreadNum) : 
        m_nPort(0), m_nReqThreadNum(reqThreadNum), m_nHandleThreadNum(processThreadNum) {

    }
    virtual ~CSimpleHttpServer() {

    }
    boost::shared_ptr<CSimpleHttpServer> GetPtrFromThis()
    {
        boost::shared_ptr<CSimpleHttpServer> p(this);
        return p;
    }
    int Init() {
        m_pServer = boost::make_shared<http::CHttpServiceImp>(m_nReqThreadNum, m_nHandleThreadNum);
        int nPort = m_pServer->StartServer(g_httpports, _countof(g_httpports));
        BD_VERIFY(nPort > 0, return -1);
        m_nPort = nPort;
        BD_VERIFY(m_pServer, return -1);
        m_pServer->Register("/version", boost::bind(&CSimpleHttpServer::HandleCheckVersion, GetPtrFromThis(), _1, _2));
        return nPort;
    }
    bool stop()
    {
        XLogDebug(L"CSimpleHttpServer::StopServer");
        if (m_pServer)
        {
            m_pServer->StopServer();
            m_pServer.reset();
        }
        return true;
    }
    int HandleCheckVersion(http::server3::request&, http::server3::reply& rep)
    {
        XLogDebug(L"HandleCheckVersion");
        rep.status = http::server3::reply::ok;
        rep.content = "{\"status\":0}";
        return 0;
    }
    int Register(const std::string& path, http_callback c)
    {
        BD_VERIFY(c, return -1);
        BD_VERIFY(m_pServer, return -1);
        return m_pServer->Register(path, c);
    }
private:
    boost::shared_ptr<http::CHttpServiceImp>m_pServer;
    long m_nPort;
    int m_nReqThreadNum;
    int m_nHandleThreadNum;

};

int HandleExit(http::server3::request&, http::server3::reply& rep) {
    XLogDebug(L"HandleExit");
    rep.status = http::server3::reply::ok;
    rep.content = "{\"status\":0}";
    io_service.stop();
    return 0;
}
int main()
{
    std::cout << "Hello World!\n";
    boost::shared_ptr<CSimpleHttpServer>pServer = boost::make_shared<CSimpleHttpServer>(2, 10);
    int nPort = pServer->Init();
    BD_VERIFY(nPort > 0, return -1);
    pServer->Register("/exit", HandleExit);
    boost::asio::io_service::work work(io_service);
    io_service.run();
    return 0;
}

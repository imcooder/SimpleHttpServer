#include "pch.h"
#include "HttpBase.h"
#include "bddebug.h"
#include "Encode.h"
#include <iostream>
#include <sstream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/thread.hpp>
using boost::asio::ip::tcp;

static std::wstring S2WS(const std::string& s)
{
    const char* pszSource = s.c_str();
    int nDSize = MultiByteToWideChar(CP_ACP, 0, pszSource, -1, NULL, 0);
    wchar_t *pszDest = new wchar_t[static_cast<size_t>(nDSize)];
    MultiByteToWideChar(CP_ACP, 0, pszSource, -1, pszDest, nDSize);
    std::wstring result = pszDest;
    delete[]pszDest;
    return result;
}
static std::string Trim(const std::string&value)
{
    std::string key = value;
    if (key.empty())
    {
        return key;
    }
    key.erase(0, key.find_first_not_of(" \t\r\n"));
    size_t nPos = key.find_last_not_of(" \t\r\n");
    if (std::wstring::npos != nPos)
    {
        key.erase(nPos + 1);
    }
    else
    {
        key.erase();
    }
    return key;
}
class CHttpClient : public boost::enable_shared_from_this < CHttpClient >
{
public:
    CHttpClient(
        boost::asio::io_service& io_service,
        const std::string& strHost,
        int nPort,
        const std::string& path,
        const std::string &type,
        const char*post,
        size_t size,
        const std::string&cookie,
        const std::string&referer,
        const std::string&contentEncode,
        const std::string accept,
        const std::string&acceptEncoding,
        const std::string userAgent,
        int nResolveTimeout, 
        int nConnectTimeout,
        int nWriteTimeout, 
        int nReadTimeout,
        const std::string&proxyServer,
        const std::string& proxyPort
        )
        : m_resolver(io_service), m_socket(io_service), m_error(false)
        , m_nResolveTimeout(nResolveTimeout), 
        m_nConnectTimeout(nConnectTimeout), 
        m_nReadTimeout(nReadTimeout), m_nWriteTimeout(nWriteTimeout)
        , m_timer(io_service)
        , m_bTimer(false)
        , m_strProxyServer(proxyServer)
        , m_strProxyPort(proxyPort)
    {
        m_strHost = strHost;
        char szPort[64] = { 0 };
        _itoa_s(nPort, szPort, _countof(szPort), 10);
        m_strPort = szPort;
        std::ostream request_stream(&m_request);
        request_stream << "POST " << path << " HTTP/1.0\r\n";
        if (m_strPort == "80")
        {
            request_stream << "Host: " << m_strHost << "\r\n";
        }
        else
        {
            request_stream << "Host: " << m_strHost << ":" << m_strPort << "\r\n";
        }
        if (accept.empty())
        {
            request_stream << "Accept: */*\r\n";
        }
        else
        {
            request_stream << "Accept: " << accept << "\r\n";
        }
        if (!acceptEncoding.empty())
        {
            request_stream << "Accept-Encoding: " << acceptEncoding << "\r\n";
        }
        if (!cookie.empty())
        {
            request_stream << "Cookie: " << cookie << "\r\n";
        }
        if (!referer.empty())
        {
            request_stream << "Referer: " << referer << "\r\n";
        }
        if (!userAgent.empty())
        {
            request_stream << "User-Agent: " << userAgent << "\r\n";
        }
        request_stream << "Content-Type: " << type << "\r\n";
        request_stream << "Content-Length: " << size << "\r\n";
        if (!contentEncode.empty())
        {
            request_stream << "Content-Encoding: " << contentEncode << "\r\n";
        }
        request_stream << "Connection: close\r\n\r\n";

        if (post && size > 0)
        {
            request_stream.write(post, size);
        }

        {
            //CONNECT www.boost.org HTTP/1.0\r\n Connection: keep-alive \r\n\r\n
            std::ostream request_stream(&m_requestProxy);
            request_stream << "CONNECT " << m_strHost << ":" << m_strPort << " HTTP/1.0\r\n";
            request_stream << "Connection: keep-alive\r\n\r\n";
        }
    }

    CHttpClient(
        boost::asio::io_service& io_service,
        const std::string& strHost,
        int nPort,
        const std::string& path,
        const std::string &type,
        const std::string&cookie,
        const std::string&referer,
        const std::string&contentEncode,
        const std::string accept,
        const std::string&acceptEncoding,
        const std::string userAgent,
        int nResolveTimeout,
        int nConnectTimeout, 
        int nWriteTimeout, 
        int nReadTimeout,
        const std::string&proxyServer,
        const std::string& proxyPort
        )
        : m_resolver(io_service),
        m_socket(io_service), m_error(false)
        , m_nResolveTimeout(nResolveTimeout),
        m_nConnectTimeout(nConnectTimeout),
        m_nReadTimeout(nReadTimeout), m_nWriteTimeout(nWriteTimeout)
        , m_timer(io_service)
        , m_bTimer(false)
        , m_strProxyServer(proxyServer)
        , m_strProxyPort(proxyPort)
    {
        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.

        m_strHost = strHost;
        char szPort[64] = { 0 };
        _itoa_s(nPort, szPort, _countof(szPort), 10);
        m_strPort = szPort;
        std::ostream request_stream(&m_request);
        request_stream << "GET " << path << " HTTP/1.0\r\n";
        if (m_strPort == "80")
        {
            request_stream << "Host: " << m_strHost << "\r\n";
        }
        else
        {
            request_stream << "Host: " << m_strHost << ":" << m_strPort << "\r\n";
        }
        if (accept.empty())
        {
            request_stream << "Accept: */*\r\n";
        }
        else
        {
            request_stream << "Accept: " << accept << "\r\n";
        }
        if (!acceptEncoding.empty())
        {
            request_stream << "Accept-Encoding: " << acceptEncoding << "\r\n";
        }
        if (!cookie.empty())
        {
            request_stream << "Cookie: " << cookie << "\r\n";
        }
        if (!referer.empty())
        {
            request_stream << "Referer: " << referer << "\r\n";
        }
        if (!userAgent.empty())
        {
            request_stream << "User-Agent: " << userAgent << "\r\n";
        }
        request_stream << "Content-Type: " << type << "\r\n";
        if (!contentEncode.empty())
        {
            request_stream << "Content-Encoding: " << contentEncode << "\r\n";
        }
        request_stream << "Connection: close\r\n\r\n";

        {
            //CONNECT www.boost.org HTTP/1.0\r\n Connection: keep-alive \r\n\r\n
            std::ostream request_stream(&m_requestProxy);
            request_stream << "CONNECT " << m_strHost << ":" << m_strPort << " HTTP/1.0\r\n";
            request_stream << "Connection: keep-alive\r\n\r\n";
        }
    }

    ~CHttpClient()
    {
        m_ss.str("");
    }
    bool run()
    {
        // Start an asynchronous resolve to translate the server and service names
        // into a list of endpoints.
        if (m_strProxyServer.empty())
        {
            tcp::resolver::query query(m_strHost, m_strPort);
            try
            {
                m_resolver.async_resolve(query, boost::bind(&CHttpClient::handle_resolve, 
                    shared_from_this(), 
                    boost::asio::placeholders::error, 
                    boost::asio::placeholders::iterator, 
                    false));
                if (m_nResolveTimeout > 0)
                {
                    m_bTimer = true;
                    m_timer.expires_from_now(boost::posix_time::milliseconds(m_nResolveTimeout));
                    m_bTimeOut = false;
                    m_timer.async_wait(boost::bind(&CHttpClient::time_out,
                        shared_from_this(), 
                        boost::asio::placeholders::error,
                        boost::ref(m_bTimeOut),
                        "resolve"));
                }
            }
            catch (boost::thread_interrupted&)
            {
                std::stringstream ss;
                ss << "resolve_error:thread interrupted " << GetCurrentThreadId();
                std::string strValue = ss.str();
                SetLastError(strValue);
                XLogError(L"%S", strValue.c_str());
            }
            catch (std::exception& e)
            {
                std::stringstream ss;
                ss << "resolve_error:Exception caught from thread " << 
                    GetCurrentThreadId() << " " << e.what();
                std::string strValue = ss.str();
                SetLastError(strValue);
                XLogError(L"%S", strValue.c_str());
            }
            catch (...)
            {
                std::stringstream ss;
                ss << "resolve_error:Unknown exception caught from thread " << 
                    GetCurrentThreadId();
                std::string strValue = ss.str();
                SetLastError(strValue);
                XLogError(L"%S", strValue.c_str());
            }
        }
        else
        {
            tcp::resolver::query query(m_strProxyServer, m_strProxyPort);
            try
            {
                m_resolver.async_resolve(query, boost::bind(&CHttpClient::handle_resolve,
                    shared_from_this(),
                    boost::asio::placeholders::error, 
                    boost::asio::placeholders::iterator,
                    true));
                if (m_nResolveTimeout > 0)
                {
                    m_bTimer = true;
                    m_timer.expires_from_now(boost::posix_time::milliseconds(m_nResolveTimeout));
                    m_bTimeOut = false;
                    m_timer.async_wait(boost::bind(&CHttpClient::time_out,
                        shared_from_this(),
                        boost::asio::placeholders::error, 
                        boost::ref(m_bTimeOut),
                        "resolve"));
                }
            }
            catch (boost::thread_interrupted&)
            {
                std::stringstream ss;
                ss << "resolve_error:thread interrupted " << GetCurrentThreadId();
                std::string strValue = ss.str();
                SetLastError(strValue);
                XLogError(L"%S", strValue.c_str());
            }
            catch (std::exception& e)
            {
                std::stringstream ss;
                ss << "resolve_error:Exception caught from thread " << 
                    GetCurrentThreadId() << " " << e.what();
                std::string strValue = ss.str();
                SetLastError(strValue);
                XLogError(L"%S", strValue.c_str());
            }
            catch (...)
            {
                std::stringstream ss;
                ss << "resolve_error:Unknown exception caught from thread " << 
                    GetCurrentThreadId();
                std::string strValue = ss.str();
                SetLastError(strValue);
                XLogError(L"%S", strValue.c_str());
            }
        }
        return true;
    }
private:
    void handle_resolve(const boost::system::error_code& err,
                        tcp::resolver::iterator endpoint_iterator, 
                        bool bProxy)
    {
        if (!err)
        {
            if (m_bTimer)
            {
                m_bTimer = false;
                m_timer.cancel();
            }
            try
            {
                boost::asio::async_connect(m_socket, endpoint_iterator, 
                                           boost::bind(&CHttpClient::handle_connect,
                    shared_from_this(), 
                    boost::asio::placeholders::error, 
                    bProxy));
                if (m_nConnectTimeout > 0)
                {
                    m_bTimer = true;
                    m_bTimeOut = false;
                    m_timer.expires_from_now(boost::posix_time::milliseconds(m_nConnectTimeout));
                    m_timer.async_wait(boost::bind(&CHttpClient::time_out, 
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::ref(m_bTimeOut), 
                        "connect"));
                }
            }
            catch (boost::thread_interrupted&)
            {
                std::stringstream ss;
                ss << "connect_error:thread interrupted " << GetCurrentThreadId();
                std::string strValue = ss.str();
                SetLastError(strValue);
                XLogError(L"%S", strValue.c_str());
            }
            catch (std::exception& e)
            {
                std::stringstream ss;
                ss << "connect_error:Exception caught from thread " << 
                    GetCurrentThreadId() << " " << e.what();
                std::string strValue = ss.str();
                SetLastError(strValue);
                XLogError(L"%S", strValue.c_str());
            }
            catch (...)
            {
                std::stringstream ss;
                ss << "connect_error:Unknown exception caught from thread " << 
                    GetCurrentThreadId();
                std::string strValue = ss.str();
                SetLastError(strValue);
                XLogError(L"%S", strValue.c_str());
            }
        }
        else
        {
            if (m_bTimeOut)
            {
                SetLastError("resolve_error:timeout");
            }
            else
            {
                SetLastError("resolve_error:" + err.message());
            }
        }
    }
    void time_out(const boost::system::error_code& error,
                  bool&bTimeOut, std::string strWho)
    {
        if (!error)
        {
            XLogDebug(L"timer expired:%s", S2WS(strWho).c_str());
            if (m_bTimer)
            {
                bTimeOut = true;
                boost::system::error_code ignored_ec;
                m_socket.close(ignored_ec);
            }
        }
        else if (error == boost::asio::error::operation_aborted)
        {
            //NET_DBG(L"timer abort or canceled");
        }
        else
        {
            XLogError(L"timer error:%s", S2WS(error.message()).c_str());
        }
    }
    void handle_connect(const boost::system::error_code& err,
                        bool bProxy)
    {
        if (!err)
        {
            if (m_bTimer)
            {
                m_bTimer = false;
                m_timer.cancel();
            }
            try
            {
                boost::asio::async_write(m_socket, !bProxy ? m_request : m_requestProxy,
                                         boost::bind(&CHttpClient::handle_write_request,
                                         shared_from_this(),
                                         boost::asio::placeholders::error, 
                                         bProxy));
                if (m_nWriteTimeout > 0)
                {
                    m_bTimer = true;
                    m_timer.expires_from_now(boost::posix_time::milliseconds(m_nWriteTimeout));
                    m_bTimeOut = false;
                    m_timer.async_wait(boost::bind(&CHttpClient::time_out, 
                        shared_from_this(), boost::asio::placeholders::error, 
                        boost::ref(m_bTimeOut), "write"));
                }
            }
            catch (boost::thread_interrupted&)
            {
                std::stringstream ss;
                ss << "write_error:thread interrupted " << GetCurrentThreadId();
                std::string strValue = ss.str();
                SetLastError(strValue);
                XLogError(L"%S", strValue.c_str());
            }
            catch (std::exception& e)
            {
                std::stringstream ss;
                ss << "write_error:Exception caught from thread " << 
                    GetCurrentThreadId() << " " << e.what();
                std::string strValue = ss.str();
                SetLastError(strValue);
                XLogError(L"%S", strValue.c_str());
            }
            catch (...)
            {
                std::stringstream ss;
                ss << "write_error:Unknown exception caught from thread " <<
                    GetCurrentThreadId();
                std::string strValue = ss.str();
                SetLastError(strValue);
                XLogError(L"%S", strValue.c_str());
            }
        }
        else
        {
            if (m_bTimeOut)
            {
                SetLastError("connect_error:timeout");
            }
            else
            {
                SetLastError("connect_error:" + err.message());
            }
        }
    }
    void handle_write_request(const boost::system::error_code& err, 
                              bool proxy)
    {
        if (!err)
        {
            // Read the response status line. The response_ streambuf will
            // automatically grow to accommodate the entire line. The growth may be
            // limited by passing a maximum size to the streambuf constructor.
            if (m_bTimer)
            {
                m_bTimer = false;
                m_timer.cancel();
            }
            if (!proxy)
            {
                try
                {
                    boost::asio::async_read_until(m_socket, m_response, "\r\n", 
                                                  boost::bind(&CHttpClient::handle_read_status_line, 
                                                  shared_from_this(), boost::asio::placeholders::error,
                                                  proxy));
                    if (m_nReadTimeout > 0)
                    {
                        m_bTimer = true;
                        m_timer.expires_from_now(boost::posix_time::milliseconds(m_nReadTimeout));
                        m_bTimeOut = false;
                        m_timer.async_wait(boost::bind(&CHttpClient::time_out, shared_from_this(), 
                            boost::asio::placeholders::error, 
                            boost::ref(m_bTimeOut), 
                            "read"));
                    }
                }
                catch (boost::thread_interrupted&)
                {
                    std::stringstream ss;
                    ss << "read_error:thread interrupted " << GetCurrentThreadId();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
                catch (std::exception& e)
                {
                    std::stringstream ss;
                    ss << "read_error:Exception caught from thread " << 
                        GetCurrentThreadId() << " " << e.what();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
                catch (...)
                {
                    std::stringstream ss;
                    ss << "read_error:Unknown exception caught from thread " <<
                        GetCurrentThreadId();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
            }
            else
            {
                try
                {
                    boost::asio::async_read_until(m_socket, m_responseProxy, "\r\n",
                                                  boost::bind(&CHttpClient::handle_read_status_line, 
                                                  shared_from_this(), 
                                                  boost::asio::placeholders::error, 
                                                  proxy));
                    if (m_nReadTimeout > 0)
                    {
                        m_bTimer = true;
                        m_timer.expires_from_now(boost::posix_time::milliseconds(m_nReadTimeout));
                        m_bTimeOut = false;
                        m_timer.async_wait(boost::bind(&CHttpClient::time_out, 
                            shared_from_this(), 
                            boost::asio::placeholders::error, 
                            boost::ref(m_bTimeOut),
                            "read"));
                    }
                }
                catch (boost::thread_interrupted&)
                {
                    std::stringstream ss;
                    ss << "read_error:thread interrupted " << GetCurrentThreadId();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
                catch (std::exception& e)
                {
                    std::stringstream ss;
                    ss << "read_error:Exception caught from thread " << 
                        GetCurrentThreadId() << " " << e.what();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
                catch (...)
                {
                    std::stringstream ss;
                    ss << "read_error:Unknown exception caught from thread " << 
                        GetCurrentThreadId();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
            }
        }
        else
        {
            if (m_bTimeOut)
            {
                SetLastError("write_error:timeout");
            }
            else
            {
                SetLastError("write_error:" + err.message());
            }
        }
    }
    void handle_read_status_line(const boost::system::error_code& err, 
                                 bool fromProxy)
    {
        if (!err)
        {
            if (m_bTimer)
            {
                m_bTimer = false;
                m_timer.cancel();
            }
            if (!fromProxy)
            {
                // Check that response is OK.
                std::istream response_stream(&m_response);
                std::string strVersion;
                response_stream >> strVersion;
                unsigned int nStatueCode = 0;
                response_stream >> nStatueCode;
                std::string strStatusMsg;
                std::getline(response_stream, strStatusMsg);
                if (!response_stream || strVersion.substr(0, 5) != "HTTP/")
                {
                    SetLastError("Invalid response");
                    return;
                }
                SetErrorCode(nStatueCode);
                if (nStatueCode != 200)
                {
                    std::stringstream s;
                    s << "Response returned with status code ";
                    s << nStatueCode << "\n";
                    SetLastError(s.str());
                    return;
                }
                // Read the response headers, which are terminated by a blank line.
                try
                {
                    boost::asio::async_read_until(m_socket, m_response, "\r\n\r\n",
                                                  boost::bind(&CHttpClient::handle_read_headers, 
                                                  shared_from_this(), 
                                                  boost::asio::placeholders::error,
                                                  fromProxy));
                    if (m_nReadTimeout > 0)
                    {
                        m_bTimer = true;
                        m_timer.expires_from_now(boost::posix_time::milliseconds(m_nReadTimeout));
                        m_bTimeOut = false;
                        m_timer.async_wait(boost::bind(&CHttpClient::time_out, 
                            shared_from_this(), 
                            boost::asio::placeholders::error, 
                            boost::ref(m_bTimeOut),
                            "read"));
                    }
                }
                catch (boost::thread_interrupted&)
                {
                    std::stringstream ss;
                    ss << "read_error:thread interrupted " << GetCurrentThreadId();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
                catch (std::exception& e)
                {
                    std::stringstream ss;
                    ss << "read_error:Exception caught from thread " << 
                        GetCurrentThreadId() << " " << e.what();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
                catch (...)
                {
                    std::stringstream ss;
                    ss << "read_error:Unknown exception caught from thread " << 
                        GetCurrentThreadId();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
            }
            else
            {
                // Check that response is OK.
                std::istream response_stream(&m_responseProxy);
                std::string strHttpVersion;
                response_stream >> strHttpVersion;
                unsigned int nStatusCode = 0;
                response_stream >> nStatusCode;
                std::string status_message;
                std::getline(response_stream, status_message);
                if (!response_stream || strHttpVersion.substr(0, 5) != "HTTP/")
                {
                    std::string strValue = "server_error:bad response";
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                    return;
                }
                if (nStatusCode != 200)
                {
                    std::stringstream ss;
                    ss << "server_error:status_code " << nStatusCode;
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                    return;
                }
                // Read the response headers, which are terminated by a blank line.
                try
                {
                    boost::asio::async_read_until(m_socket, m_responseProxy, "\r\n\r\n", 
                                                  boost::bind(&CHttpClient::handle_read_headers,
                                                  shared_from_this(), 
                                                  boost::asio::placeholders::error, 
                                                  fromProxy));
                    if (m_nReadTimeout > 0)
                    {
                        m_bTimer = true;
                        m_timer.expires_from_now(boost::posix_time::milliseconds(m_nReadTimeout));
                        m_bTimeOut = false;
                        m_timer.async_wait(boost::bind(&CHttpClient::time_out, 
                            shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::ref(m_bTimeOut),
                            "read"));
                    }
                }
                catch (boost::thread_interrupted&)
                {
                    std::stringstream ss;
                    ss << "read_error:thread interrupted " << GetCurrentThreadId();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
                catch (std::exception& e)
                {
                    std::stringstream ss;
                    ss << "read_error:Exception caught from thread " << 
                        GetCurrentThreadId() << " " << e.what();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
                catch (...)
                {
                    std::stringstream ss;
                    ss << "read_error:Unknown exception caught from thread " << 
                        GetCurrentThreadId();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
            }
        }
        else
        {
            if (m_bTimeOut)
            {
                SetLastError("read_error:timeout");
            }
            else
            {
                SetLastError("read_error:" + err.message());
            }
        }
    }

    void handle_read_headers(const boost::system::error_code& err, 
                             bool fromProxy)
    {
        if (!err)
        {
            // Process the response headers.
            if (!fromProxy)
            {
                std::istream response_stream(&m_response);
                std::string header;
                while (std::getline(response_stream, header) && header != "\r")
                {
                    AddReponseHeader(header);
                }
                // Write whatever content we already have to output.
                if (m_response.size() > 0)
                {
                    m_ss << &m_response;
                }
                // Start reading remaining data until EOF.				
                try
                {
                    boost::asio::async_read(m_socket, m_response, boost::asio::transfer_at_least(1),
                                            boost::bind(&CHttpClient::handle_read_content, shared_from_this(),
                                            boost::asio::placeholders::error, 
                                            fromProxy));
                    if (m_nReadTimeout > 0)
                    {
                        m_bTimer = true;
                        m_timer.expires_from_now(boost::posix_time::milliseconds(m_nReadTimeout));
                        m_bTimeOut = false;
                        m_timer.async_wait(boost::bind(&CHttpClient::time_out, 
                            shared_from_this(), 
                            boost::asio::placeholders::error,
                            boost::ref(m_bTimeOut), 
                            "read"));
                    }
                }
                catch (boost::thread_interrupted&)
                {
                    std::stringstream ss;
                    ss << "read_error:thread interrupted " << GetCurrentThreadId();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
                catch (std::exception& e)
                {
                    std::stringstream ss;
                    ss << "read_error:Exception caught from thread " << 
                        GetCurrentThreadId() << " " << e.what();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
                catch (...)
                {
                    std::stringstream ss;
                    ss << "read_error:Unknown exception caught from thread " << 
                        GetCurrentThreadId();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
            }
            else
            {
                std::istream response_stream(&m_responseProxy);
                std::string header;
                while (std::getline(response_stream, header) && header != "\r")
                {
                    //std::cout << header << "\n";
                }
                //std::cout << "\n";
                // Write whatever content we already have to output.
                if (m_responseProxy.size() > 0)
                {
                    //m_ss << &response_;
                }
                //代理没有content数据
                try
                {
                    boost::asio::async_write(m_socket, m_request, 
                                             boost::bind(&CHttpClient::handle_write_request, 
                                             shared_from_this(), 
                                             boost::asio::placeholders::error,
                                             false));
                    /*
                    boost::asio::async_read(socket_, proxy_response_, boost::asio::transfer_at_least(1),
                    boost::bind(&http_client::handle_read_content, shared_from_this(), 
                    boost::asio::placeholders::error, fromProxy));
                    */
                    if (m_nWriteTimeout > 0)
                    {
                        m_bTimer = true;
                        m_timer.expires_from_now(boost::posix_time::milliseconds(m_nWriteTimeout));
                        m_bTimeOut = false;
                        m_timer.async_wait(boost::bind(&CHttpClient::time_out, 
                            shared_from_this(), 
                            boost::asio::placeholders::error,
                            boost::ref(m_bTimeOut),
                            "write"));
                    }
                }
                catch (boost::thread_interrupted&)
                {
                    std::stringstream ss;
                    ss << "write_error:thread interrupted  " << GetCurrentThreadId();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
                catch (std::exception& e)
                {
                    std::stringstream ss;
                    ss << "write_error:Exception caught from thread " << 
                        GetCurrentThreadId() << " " << e.what();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
                catch (...)
                {
                    std::stringstream ss;
                    ss << "write_error:Unknown exception caught from thread " << 
                        GetCurrentThreadId();
                    std::string strValue = ss.str();
                    SetLastError(strValue);
                    XLogError(L"%S", strValue.c_str());
                }
            }
        }
        else
        {
            if (m_bTimeOut)
            {
                SetLastError("read_error:timeout");
            }
            else
            {
                SetLastError("read_error:" + err.message());
            }
        }
    }

    void handle_read_content(const boost::system::error_code& err, bool fromProxy)
    {
        if (!err)
        {
            if (m_bTimer)
            {
                m_bTimer = false;
                m_timer.cancel();
            }
            // Write all of the data that has been read so far.
            m_ss << &m_response;
            try
            {
                boost::asio::async_read(m_socket, m_response, boost::asio::transfer_at_least(1), 
                                        boost::bind(&CHttpClient::handle_read_content, 
                                        shared_from_this(), 
                                        boost::asio::placeholders::error, 
                                        fromProxy));
                if (m_nReadTimeout > 0)
                {
                    m_bTimer = true;
                    m_timer.expires_from_now(boost::posix_time::milliseconds(m_nReadTimeout));
                    m_bTimeOut = false;
                    m_timer.async_wait(boost::bind(&CHttpClient::time_out,
                        shared_from_this(),
                        boost::asio::placeholders::error, 
                        boost::ref(m_bTimeOut), 
                        "read"));
                }
            }
            catch (boost::thread_interrupted&)
            {
                std::stringstream ss;
                ss << "read_error:thread interrupted " << GetCurrentThreadId();
                std::string strValue = ss.str();
                SetLastError(strValue);
                XLogError(L"%S", strValue.c_str());
            }
            catch (std::exception& e)
            {
                std::stringstream ss;
                ss << "read_error:Exception caught from thread " <<
                    GetCurrentThreadId() << " " << e.what();
                std::string strValue = ss.str();
                SetLastError(strValue);
                XLogError(L"%S", strValue.c_str());
            }
            catch (...)
            {
                std::stringstream ss;
                ss << "read_error:Unknown exception caught from thread " << 
                    GetCurrentThreadId();
                std::string strValue = ss.str();
                SetLastError(strValue);
                XLogError(L"%S", strValue.c_str());
            }
        }
        else if (err == boost::asio::error::eof)
        {
            if (m_bTimer)
            {
                m_bTimer = false;
                m_timer.cancel();
            }
        }
        else if (err != boost::asio::error::eof)
        {
            if (m_bTimeOut)
            {
                SetLastError("read_error:timeout");
            }
            else
            {
                SetLastError("read_error:" + err.message());
            }
        }
    }

    void SetLastError(const std::string &error)
    {
        m_strError = error;
        m_error = !m_strError.empty();
        //XLogError(L"Error:%S", error.c_str());        
    }
    void SetErrorCode(int nValue)
    {
        m_nErrorCode = nValue;
    }
    void ClearError()
    {
        m_nErrorCode = 200;
        m_strError.clear();
        m_error = !m_strError.empty();
    }
public:
    std::string GetResult()
    {
        return m_ss.str();
    }
    bool Error()
    {
        return m_error;
    }
    std::string GetError()
    {
        return m_strError;
    }
    int GetErrorCode()
    {
        return m_nErrorCode;
    }
    http::HEADER_MAP GetResponseHeader()
    {
        return m_mapResponseHeader;
    }
private:
    void AddReponseHeader(const std::string&header)
    {
        std::string::size_type pos = header.find(":");
        if (pos == std::string::npos)
        {
            return;
        }
        std::string key = Trim(header.substr(0, pos));
        if (key.empty())
        {
            return;
        }
        std::string value = Trim(header.substr(pos + 1));
        if (value.empty())
        {
            return;
        }
        m_mapResponseHeader[key] = value;
    }
private:
    tcp::resolver m_resolver;
    tcp::socket m_socket;
    boost::asio::streambuf m_request;
    boost::asio::streambuf m_requestProxy;
    boost::asio::streambuf m_response;
    boost::asio::streambuf m_responseProxy;
    std::stringstream m_ss;
    http::HEADER_MAP m_mapResponseHeader;
    bool m_error;
    std::string m_strError;
    int m_nErrorCode;
    int m_nResolveTimeout;
    int  m_nConnectTimeout;
    int m_nReadTimeout;
    int m_nWriteTimeout;
    boost::asio::deadline_timer m_timer;
    bool m_bTimer;
    bool m_bTimeOut;
    std::string m_strProxyServer;
    std::string m_strProxyPort;
    std::string m_strHost;
    std::string m_strPort;
};


namespace http
{
    CHttpBase::CHttpBase()
        : m_nConnectTimeout(1000),
        m_nResolveTimeout(1000),
        m_nWriteTimeout(5 * 1000), 
        m_nReadTimeout(5 * 1000)
    {
        //m_strProxyServer = "127.0.0.1";
        //m_strProxyPort = "8888";
    }
    CHttpBase::~CHttpBase(void)
    {
    }
    void CHttpBase::SetConnectTimeOut(int timeout)
    {
        m_nConnectTimeout = timeout;
        return;
    }
    void CHttpBase::SetResolveTimeout(int timeout)
    {
        m_nResolveTimeout = timeout;
    }
    void CHttpBase::SetWriteTimeout(int timeout)
    {
        m_nWriteTimeout = timeout;
    }

    void CHttpBase::SetReadTimeout(int timeout)
    {
        m_nReadTimeout = timeout;
    }

    bool CHttpBase::Get(const std::string &url, 
                        const std::string &type)
    {
        bool bError = false;
        std::string host;
        std::string uri;
        m_result.clear();
        m_mapResponseHeader.clear();
        SetLastError("");
        try
        {
            std::string arg;
            int nPort = 80;
            Util::URL::Parse(url, NULL, &host, &uri, &nPort, &arg);
            if (!arg.empty())
            {
                uri += "?" + arg;
            }
            boost::asio::io_service ioService;
            boost::shared_ptr<CHttpClient> pClient(new(std::nothrow) CHttpClient(
                ioService, host, nPort, uri, type, m_strCookie, m_strReferer,
                m_strContentEncode, m_strAccept, m_strAcceptEncode,
                m_strUserAgent, m_nResolveTimeout, m_nConnectTimeout, 
                m_nWriteTimeout, m_nReadTimeout, m_strProxyServer, 
                m_strProxyPort));
            BD_VERIFY(pClient, return false);
            pClient->run();
            boost::system::error_code ignoredEc;
            ioService.run(ignoredEc);
            if (ignoredEc)
            {
                bError = true;
                BD_VERIFY(0, return false);
            }
            if (pClient->Error())
            {
                SetLastError(pClient->GetError());
                bError = true;
            }
            else
            {
                m_result = pClient->GetResult();
                m_mapResponseHeader = pClient->GetResponseHeader();
                bError = false;
            }
        }
        catch (boost::thread_interrupted&)
        {
            std::stringstream ss;
            ss << "Thread  " << GetCurrentThreadId() << " interrupted";
            std::string strValue = ss.str();
            SetLastError(strValue);
            XLogError(L"%S", strValue.c_str());
            bError = true;
        }
        catch (std::exception& e)
        {
            std::stringstream ss;
            ss << "Exception caught from thread " << GetCurrentThreadId() << " " << e.what();
            std::string strValue = ss.str();
            SetLastError(strValue);
            XLogError(L"%S", strValue.c_str());
            bError = true;
        }
        catch (...)
        {
            std::stringstream ss;
            ss << "Unknown exception caught from thread " << GetCurrentThreadId();
            std::string strValue = ss.str();
            SetLastError(strValue);
            XLogError(L"%S", strValue.c_str());
            bError = true;
        }
        return !bError;
    }

    bool CHttpBase::Post(const std::string &url, const std::string &post, const std::string &type)
    {
        bool bError = false;
        std::string host;
        std::string uri;
        m_result.clear();
        m_mapResponseHeader.clear();
        SetLastError("");
        try
        {
            std::string arg;
            int nPort = 80;
            Util::URL::Parse(url, NULL, &host, &uri, &nPort, &arg);
            if (!arg.empty())
            {
                uri += "?" + arg;
            }
            boost::asio::io_service ioService;
            boost::shared_ptr<CHttpClient> pClient(new(std::nothrow) CHttpClient(
                ioService, host, nPort, uri, type, post.c_str(), post.size(),
                m_strCookie, m_strReferer, m_strContentEncode, m_strAccept, 
                m_strAcceptEncode, 
                m_strUserAgent, m_nResolveTimeout, m_nConnectTimeout, 
                m_nWriteTimeout, m_nReadTimeout
                , m_strProxyServer, 
                m_strProxyPort));
            BD_VERIFY(pClient, return false);
            pClient->run();
            boost::system::error_code ignoredEc;
            ioService.run(ignoredEc);
            if (ignoredEc)
            {
                bError = false;
                BD_VERIFY(0, return false);
            }
            if (pClient->Error())
            {
                m_strError = pClient->GetError();
                bError = true;
            }
            else
            {
                m_result = pClient->GetResult();
                m_mapResponseHeader = pClient->GetResponseHeader();
                bError = false;
            }
        }
        catch (boost::thread_interrupted&)
        {
            std::stringstream ss;
            ss << "Thread  " << GetCurrentThreadId() << " interrupted";
            std::string strValue = ss.str();
            SetLastError(strValue);
            XLogError(L"%S", strValue.c_str());
            bError = true;
        }
        catch (std::exception& e)
        {
            std::stringstream ss;
            ss << "Exception caught from thread " << 
                GetCurrentThreadId() << " " << e.what();
            std::string strValue = ss.str();
            SetLastError(strValue);
            XLogError(L"%S", strValue.c_str());
            bError = true;
        }
        catch (...)
        {
            std::stringstream ss;
            ss << "Unknown exception caught from thread " << 
                GetCurrentThreadId();
            std::string strValue = ss.str();
            SetLastError(strValue);
            XLogError(L"%S", strValue.c_str());
            bError = true;
        }
        return !bError;
    }
    std::string CHttpBase::GetResult()
    {
        return m_result;
    }
    std::string CHttpBase::GetLastError()
    {
        return m_strError;
    }

    void CHttpBase::SetReferer(const char*pszValue)
    {
        if (pszValue && *pszValue)
        {
            m_strReferer = pszValue;
        }
        else
        {
            m_strReferer.clear();
        }
    }

    void CHttpBase::SetCookie(const char*pszValue)
    {
        if (pszValue && *pszValue)
        {
            m_strCookie = pszValue;
        }
        else
        {
            m_strCookie.clear();
        }
    }

    void CHttpBase::SetContentEncode(const std::string& encode)
    {
        m_strContentEncode = encode;
        return;
    }

    void CHttpBase::SetAcceptEncode(const std::string&value)
    {
        m_strAcceptEncode = value;
    }

    void CHttpBase::SetProxy(const std::string&strProxy, 
                             const std::string& strPort)
    {
        m_strProxyServer = strProxy;
        m_strProxyPort = strPort;
        return;
    }

    void CHttpBase::SetLastError(const std::string&err)
    {
        m_strError = err;
    }

    std::string CHttpBase::GetResponseHeader(const std::string&name,
                                        const std::string&def /*= ""*/)
    {
        auto it = m_mapResponseHeader.find(name);
        if (it == m_mapResponseHeader.end())
        {
            return def;
        }
        if (it->second.empty())
        {
            return def;
        }
        return it->second;
    }

    http::HEADER_MAP CHttpBase::GetResponseHeader()
    {
        return m_mapResponseHeader;
    }

    void CHttpBase::SetAccept(const std::string& value)
    {
        m_strAccept = value;
        return;
    }

    void CHttpBase::SetUserAgent(const std::string&value)
    {
        m_strUserAgent = value;
        return;
    }

    int CHttpBase::GetErrorCode()
    {
        return m_nErrorCode;
    }

    void CHttpBase::SetErrorCode(int nValue)
    {
        m_nErrorCode = nValue;
    }

};
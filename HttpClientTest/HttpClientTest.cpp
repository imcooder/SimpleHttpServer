// HttpClientTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <string>
#include <iostream>
#include "HttpLib/HttpBase.h"


int get() {
    std::string url = "http://127.0.0.1:5809/version";
    http::CHttpBase net;
    net.SetConnectTimeOut(2000);
    if (net.Get(url, HTTP_CONNECT_TYPE_JSON)) {
        std::string res = net.GetResult();
        std::cout << res << std::endl;
    }
    else {
        std::cout << "error: " << net.GetLastError() << std::endl;
    }
    return 0;
}
int main()
{
    for (int i = 0; i < 10; i++) {
        get();
    }
}


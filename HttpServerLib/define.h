#pragma once
#include "HttpServerLib/request.hpp"
#include "HttpServerLib/reply.hpp"

#ifndef _HTTP_CALLBACK_DEFINED_
#define _HTTP_CALLBACK_DEFINED_
	typedef boost::function<HRESULT (http::server3::request&req, http::server3::reply& rep)> http_callback;
#endif


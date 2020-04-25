#pragma once
#include <string>
#include <unordered_map>
namespace http
{
    struct Hash_Key
    {
        size_t operator()(const std::string &key) const
        {
            std::string strTmp(key);
            struct TOLOW
            {
                void operator()(char& ch)
                {
                    if ((ch >= (char)'A') && (ch <= (char)'Z'))
                    {
                        ch = (char)(ch - 'A' + 'a');
                    }
                }
            };
            std::for_each(strTmp.begin(), strTmp.end(), TOLOW());
            typedef std::hash<std::string> HASH;
            return HASH()(strTmp);
        }
    };
    struct Equal_Key
    {
        bool operator()(const std::string &left, const std::string &right) const
        {
            return !_stricmp(left.c_str(), right.c_str());
        }
    };
    typedef std::unordered_map<std::string, std::string, Hash_Key, Equal_Key> HEADER_MAP;


    enum HTTP_ERROR
    {
        OK = 200,
        CREATED = 201,
        ACCEPTED = 202,
        NO_CONTENT = 204,
        MULTIPLE_CHOICES = 300,
        MOVED_PERMANENTLY = 301,
        MOVED_TEMPORARILY = 302,
        NOT_MODIFIED = 304,
        BAD_REQUEST = 400,
        UNAUTHORIZED = 401,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        INTERNAL_SERVER_ERROR = 500,
        NOT_IMPLEMENTED = 501,
        BAD_GATEWAY = 502,
        SERVICE_UNAVAILABLE = 503
    };
};
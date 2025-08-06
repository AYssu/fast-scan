#pragma once
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>


namespace shttp {

    struct http_response {
        bool success;     // true if the request was successful, false otherwise
        long code;         // the HTTP status code
        std::string data; // the response data
        std::string error;// the error message
        std::string  content_type; // the content type
    };
    // example usage: http_xxx("你的域名,可带https://头也可不带","请求参数","自定义请求头,自行了解规范",true/*就使用https了需要服务器支持*/,true/*是否打印错误日志*/);
    http_response http_get(const std::string &url, const std::string &header = "Content-Type: application/x-www-form-urlencoded", bool log = false);// send a GET request to the specified URL and return the response

    http_response http_post(const std::string &url, const std::string &params = "", const std::string &header = "Content-Type: application/x-www-form-urlencoded", bool log = false);// send a GET request to the specified URL and return the response

}// namespace shttp

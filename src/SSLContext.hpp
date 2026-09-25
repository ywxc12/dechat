#include "Config.hpp"
#pragma once
class SSLContext{
    ssl::context ctx;
    public:
    SSLContext(
        std::string caname,
        std::string pkeyname,
        ssl::context::file_format fmt):ctx(
        ssl::context::tlsv12_server)
    {
        ctx.use_certificate_file(caname,fmt);
        ctx.use_private_key_file(pkeyname, fmt);
        ctx.set_verify_mode(ssl::context::verify_none);
    }
    operator ssl::context&(){
        return ctx;
    }
};
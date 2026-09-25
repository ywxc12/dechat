#include<boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/multicast.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/unicast.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/verify_context.hpp>
#include <boost/asio/ssl/verify_mode.hpp>
#include <boost/asio/strand.hpp>
#include<boost/beast.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <QJsonObject>
#include <cstdint>
#include <memory>
#include <qcborvalue.h>
#include <qdebug.h>
#include <qhashfunctions.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qsharedpointer.h>
#include <QFile>
#include <qtmetamacros.h>
#include <qlogging.h>
#include <nlohmann/json.hpp>
#include <boost/asio/ssl.hpp>
#include <vector>
#define LOGFMT(Status,Message) \
(std::string(__FILE__)+__PRETTY_FUNCTION__+#Status+#Message)
#pragma once
namespace net=boost::asio;
namespace ip = net::ip;
namespace beast=boost::beast;
namespace websocket = boost::beast::websocket;
namespace ssl=net::ssl;
using ip::tcp;
using ip::udp;
using wss=websocket::stream<ssl::stream<tcp::socket>>;
using Json=nlohmann::json;

namespace boost::asio{
    mutable_buffer buffer(QByteArray &array){
        return buffer(array.begin(),array.size());
    }
}
QByteArray Pack(Json j){
    auto xxs=Json::to_cbor(j);
    QByteArray bas;
    for (auto i : xxs) {
        bas.push_back(i);
    }
    return bas;
}
Json Unpack(QByteArray bytea){
    std::vector<std::uint8_t>data(bytea.begin(),bytea.end());
    return Json::from_cbor(data);
}
template<typename T>using sptr=std::shared_ptr<T>;
class WSSStream{
    sptr<wss> socket;
    public:
    WSSStream(net::io_context& ioc,ssl::context& ctx){
        socket=std::make_shared<wss>(ioc,ctx);
    }
};
class Context{
    WSSStream wss;
    net::io_context ioc;
    ssl::context ctx;
    public:
    Context():ctx(ssl::context::tls_client),wss(ioc,ctx){
        
    }
};

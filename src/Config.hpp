// #include "CoreControl.hpp"
#pragma once
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl/verify_mode.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/logic/tribool.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <boost/beast.hpp>
#include <boost/asio/ssl.hpp>
#include <shared_mutex>
using namespace std;
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
using TcpSSLSocket=ssl::stream<tcp::socket>;
using wss=websocket::stream<ssl::stream<tcp::socket>>;
using Json=nlohmann::json;
using std::shared_ptr;

#pragma once
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <deque>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace net = boost::asio;
namespace ssl = net::ssl;
namespace beast = boost::beast;
namespace http = beast::http;
using tcp = net::ip::tcp;

using SSLStream = ssl::stream<tcp::socket>;
using SSLWebSocket = beast::websocket::stream<SSLStream>;

class WSSSession : public std::enable_shared_from_this<WSSSession> {
public:
    WSSSession(tcp::socket socket, ssl::context& ctx);
    virtual ~WSSSession() = default;

    net::awaitable<void> server_accept();
    void send_text(std::string msg);
    void send_binary(std::vector<uint8_t> data);
    
    // Exposed for external force close (e.g., admin kick)
    void force_close() { close(); }

protected:
    virtual void on_message(const std::string& msg) {}
    virtual void on_binary(const std::vector<uint8_t>& data) {}
    virtual void on_disconnect() {}
    virtual net::awaitable<bool> authenticate(const std::string& session_id) = 0;

    SSLWebSocket ws_;
    beast::flat_buffer read_buffer_;
    std::deque<std::variant<std::string, std::vector<uint8_t>>> send_queue_;
    bool is_writing_ = false;

    std::string parse_cookie(const std::string& cookie_str, const std::string& key);
    net::awaitable<void> read_loop();
    void enqueue(std::variant<std::string, std::vector<uint8_t>> data, bool is_binary);
    void do_write(bool is_binary);
    void close();
};
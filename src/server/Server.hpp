#include "boost/asio/detached.hpp"
#include<boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>
#include <map>
#include <deque>
#include <variant>
#include <memory>
#include <nlohmann/json.hpp>

namespace net = boost::asio;
namespace ip = net::ip;
using tcp = ip::tcp;
namespace ssl = net::ssl;
namespace beast = boost::beast;
using Json = nlohmann::json;

using SSLStream = ssl::stream<tcp::socket>;
using SSLWebSocket = beast::websocket::stream<SSLStream>;

// ================= 底层：WSSSession 封装 =================
class WSSSession : public std::enable_shared_from_this<WSSSession> {
public:
    WSSSession(tcp::socket socket, ssl::context& ctx)
        : ws_(std::move(socket), ctx) {}
    virtual ~WSSSession() = default;

    net::awaitable<void> server_accept() {
        try {
            // 1. TLS 握手
            co_await ws_.next_layer().async_handshake(ssl::stream_base::server, net::use_awaitable);
            // 2. WebSocket 握手
            co_await ws_.async_accept(net::use_awaitable);
            
            // 3. 启动接收循环协程
            net::co_spawn(ws_.get_executor(), read_loop(), net::detached);
        } catch (const std::exception& e) {
            std::cerr << "[WSS] 握手失败: " << e.what() << "\n";
            close();
        }
    }

    void send_text(std::string msg) { enqueue(std::move(msg), false); }
    void send_binary(std::vector<uint8_t> data) { enqueue(std::move(data), true); }

protected:
    virtual void on_message(const std::string& msg) {}
    virtual void on_binary(const std::vector<uint8_t>& data) {}
    virtual void on_disconnect() {}

    SSLWebSocket ws_;
    beast::flat_buffer buffer_;
    
    std::deque<std::variant<std::string, std::vector<uint8_t>>> send_queue_;
    bool is_writing_ = false;

    net::awaitable<void> read_loop() {
        try {
            for (;;) {
                buffer_.consume(buffer_.size());
                co_await ws_.async_read(buffer_, net::use_awaitable);

                std::string msg = beast::buffers_to_string(buffer_.data());
                if (ws_.got_binary()) {
                    std::vector<uint8_t> bin_data(msg.begin(), msg.end());
                    on_binary(bin_data);
                } else {
                    on_message(msg);
                }
            }
        } catch (const std::exception& e) {
            close();
        }
    }

    void enqueue(std::variant<std::string, std::vector<uint8_t>> data, bool is_binary) {
        send_queue_.push_back(std::move(data));
        if (!is_writing_) do_write(is_binary);
    }

        void do_write(bool is_binary) {
        if (send_queue_.empty()) {
            is_writing_ = false;
            return;
        }
        is_writing_ = true;
        
        // 【关键修改1】取出数据后立刻弹出队列，避免后续逻辑修改队列
        auto data = std::move(send_queue_.front());
        send_queue_.pop_front();

        ws_.binary(is_binary);

        // 【关键修改2】把整个 variant 装进 shared_ptr，这样它的生命周期就交给了协程捕获
        auto buffer_ptr = std::make_shared<std::variant<std::string, std::vector<uint8_t>>>(std::move(data));
        
        // 【关键修改3】强转返回类型为 net::const_buffer，保证 std::visit 返回类型绝对一致！
        net::const_buffer bbuf = std::visit([](auto& d) -> net::const_buffer { 
            return net::buffer(d); 
        }, *buffer_ptr);

        net::co_spawn(ws_.get_executor(), 
            [this, buffer_ptr, bbuf]() -> net::awaitable<void> {
                try {
                    co_await ws_.async_write(bbuf, net::use_awaitable);
                    do_write(false); // 发送完毕，继续发下一个
                } catch (...) {
                    close();
                }
            }, net::detached);
    }

    void close() {
        if (ws_.is_open()) {
            beast::error_code ec;
            ws_.close(beast::websocket::close_code::normal, ec);
        }
        on_disconnect();
    }
};

// ================= 业务层：前置声明 =================
class Server; 

class ChatSession : public WSSSession {
public:
    ChatSession(tcp::socket socket, ssl::context& ctx, Server* server, std::string client_id)
        : WSSSession(std::move(socket), ctx), server_(server), id_(std::move(client_id)) {}

    const std::string& get_id() const { return id_; }

    void on_message(const std::string& msg) override;
    void on_disconnect() override;

private:
    Server* server_;
    std::string id_;
};

// ================= 服务器主体 =================
class Server {
    ssl::context ctx;
    net::io_context ioc;
    tcp::acceptor acceptor_;
    
    // 存储在线用户的 Session
    std::map<std::string, std::shared_ptr<ChatSession>> users;

    void load_certificates(std::string cert, std::string key) {
        try {
            ctx.use_certificate_chain_file(cert);
            ctx.use_private_key_file(key, ssl::context::pem);
            ctx.set_options(
                ssl::context::default_workarounds |
                ssl::context::no_sslv2 |
                ssl::context::no_sslv3 |
                ssl::context::single_dh_use);
            std::cout << "[TLS] 证书与私钥加载成功！\n";
        } catch (const std::exception &e) {
            std::cerr << "[TLS] 证书加载失败: " << e.what() << "\n";
            std::exit(EXIT_FAILURE);
        }
    }

    net::awaitable<void> to_accept() {
        for (;;) {
            try {
                tcp::socket socket = co_await acceptor_.async_accept(net::use_awaitable);
                
                // 获取客户端标识（IP:端口）
                std::string client_id = socket.remote_endpoint().address().to_string() + ":" + 
                                        std::to_string(socket.remote_endpoint().port());
                std::cout << "新 TCP 客户端接入: " << client_id << "\n";

                // 创建 ChatSession 并入网
                auto session = std::make_shared<ChatSession>(std::move(socket), ctx, this, client_id);
                users[client_id] = session; 
                
                // 启动握手
                net::co_spawn(ioc, session->server_accept(), net::detached);
            } catch (const boost::system::system_error &e) {
                if (e.code() == net::error::operation_aborted) break;
                std::cerr << "Accept 异常: " << e.what() << "\n";
            }
        }
    }

public:
    Server(short port, std::string cf, std::string kf)
        : acceptor_(ioc, tcp::endpoint(tcp::v4(), port)),
          ctx(ssl::context::tlsv13_server) {
        load_certificates(cf, kf);
    }

    void run() {
        net::co_spawn(ioc, to_accept(), net::detached);
        ioc.run();
    }

    // 广播接口（无视发送者，发给其他人）
    void broadcast(const std::string& msg, const std::string& sender_id) {
        for (auto& [id, session] : users) {
            if (id != sender_id) {
                session->send_text(msg);
            }
        }
    }

    void remove_session(const std::string& id) {
        users.erase(id);
        std::cout << "[" << id << "] 已下线，当前在线人数: " << users.size() << "\n";
    }
};

// ================= ChatSession 业务实现 =================
void ChatSession::on_message(const std::string& msg) {
    std::cout << "收到 [" << id_ << "] 消息: " << msg << "\n";
    
    try {
        // 尝试解析为 JSON，加上发送者的 ID 再广播出去
        Json j = Json::parse(msg);
        j["from"] = id_; 
        server_->broadcast(j.dump(), id_);
    } catch (const Json::exception& e) {
        // 如果客户端乱发非 JSON 文本，就提示错误
        std::cerr << "JSON 解析失败: " << e.what() << "\n";
        send_text("{\"error\": \"invalid json\"}");
    }
}

void ChatSession::on_disconnect() {
    server_->remove_session(id_);
}
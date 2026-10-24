#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <limits>
#include <functional>
#include <algorithm>

// 基础版：只获取字符串，带默认值
std::string getInputStr(const std::string& prompt, const std::string& defaultValue) {
    std::cout << prompt << " [" << defaultValue << "]: ";
    std::string input;
    
    // 如果读取失败（比如 EOF），直接返回默认值
    if (!std::getline(std::cin, input)) {
        return defaultValue;
    }

    // 去除首尾空白字符（防止用户只敲了一个空格）
    input.erase(0, input.find_first_not_of(" \t\r\n"));
    input.erase(input.find_last_not_of(" \t\r\n") + 1);

    // 如果用户直接敲回车，返回默认值
    if (input.empty()) {
        return defaultValue;
    }
    return input;
}

// 进阶版：泛型模板，支持输入数字、浮点数，并带有合法性校验
template <typename T>
T getInput(const std::string& prompt, const T& defaultValue, 
           std::function<T(const std::string&)> converter) {
    while (true) {
        std::cout << prompt << " [" << defaultValue << "]: ";
        std::string input;
        
        if (!std::getline(std::cin, input)) {
            return defaultValue; // EOF 处理
        }
        
        // 去除空白
        input.erase(0, input.find_first_not_of(" \t\r\n"));
        input.erase(input.find_last_not_of(" \t\r\n") + 1);

        if (input.empty()) {
            return defaultValue;
        }

        try {
            // 尝试用传入的转换器（如 std::stoi）进行转换
            return converter(input);
        } catch (const std::exception&) {
            // 如果转换失败（比如输入了字母要求数字），提示并重新循环
            std::cerr << "  [错误] 输入格式不合法，请重新输入！\n";
        }
    }
}
#include <nlohmann/json.hpp>
using Json=nlohmann::json;

class Message{
    Json jll;
    Json ConstructFileJson(std::string fileName){
        std::ifstream ifs;
        ifs.open(fileName);
        std::stringstream ss;
        ss<<ifs.rdbuf();
        Json pl;
        pl["Data"]=ss.str();
        pl["FileName"]=fileName;
        return pl.dump();
    }
    public:
};
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <deque>
#include <iostream>
#include <memory>
#include <string>

namespace net = boost::asio;
namespace ssl = net::ssl;
namespace beast = boost::beast;
using tcp = net::ip::tcp;

using SSLWebSocket = beast::websocket::stream<ssl::stream<tcp::socket>>;

class WebSocketTLSSession : public std::enable_shared_from_this<WebSocketTLSSession> {
public:
    WebSocketTLSSession(tcp::socket socket, ssl::context& ctx)
        : ws_(std::move(socket), ctx) {}

    // ==================== 服务端：双重握手 ====================
    net::awaitable<void> server_accept() {
        try {
            // 1. TLS 握手
            co_await ws_.next_layer().async_handshake(ssl::stream_base::server, net::use_awaitable);
            // 2. WebSocket 握手
            co_await ws_.async_accept(net::use_awaitable);
            std::cout << "[WSS] 服务端握手成功！\n";
            
            // 握手成功后，启动接收循环
            net::co_spawn(ws_.get_executor(), read_loop(), net::detached);
        } catch (const std::exception& e) {
            std::cerr << "[WSS] 握手失败: " << e.what() << "\n";
            close();
        }
    }

    // ==================== 客户端：双重握手 ====================
    net::awaitable<void> client_connect(const std::string& host, const std::string& port) {
        try {
            // 1. 解析并连接 TCP
            tcp::resolver resolver(ws_.get_executor());
            auto endpoints = co_await resolver.async_resolve(host, port, net::use_awaitable);
            co_await net::async_connect(ws_.next_layer().next_layer(), endpoints, net::use_awaitable);

            // 2. 设置 SNI（极其重要）
            if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host.c_str())) {
                throw std::runtime_error("SNI 设置失败");
            }

            // 3. TLS 握手
            co_await ws_.next_layer().async_handshake(ssl::stream_base::client, net::use_awaitable);

            // 4. WebSocket 握手
            co_await ws_.async_handshake(host, "/", net::use_awaitable);
            std::cout << "[WSS] 客户端连接成功！\n";

            // 启动接收循环
            net::co_spawn(ws_.get_executor(), read_loop(), net::detached);
        } catch (const std::exception& e) {
            std::cerr << "[WSS] 客户端连接失败: " << e.what() << "\n";
            close();
        }
    }

    // ==================== 发送接口（入队） ====================
    void send_text(std::string msg) {
        enqueue(std::move(msg), false);
    }

    void send_binary(std::vector<uint8_t> data) {
        enqueue(std::move(data), true);
    }

    // ==================== 接收回调（由业务层继承并重写） ====================
    virtual void on_message(const std::string& msg) {}
    virtual void on_binary(const std::vector<uint8_t>& data) {}
    virtual void on_disconnect() {}

protected:
    SSLWebSocket ws_;
    beast::flat_buffer buffer_;
    
    // 发送队列控制
    std::deque<std::variant<std::string, std::vector<uint8_t>>> send_queue_;
    bool is_writing_ = false;

    // 接收循环协程
    net::awaitable<void> read_loop() {
        try {
            for (;;) {
                buffer_.consume(buffer_.size()); // 清理上一帧的数据
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
            // 客户端断开或网络错误
            close();
        }
    }

    // 核心发送逻辑：保证同一时刻只有一个 async_write
    void enqueue(std::variant<std::string, std::vector<uint8_t>> data, bool is_binary) {
        send_queue_.push_back(std::move(data));
        if (!is_writing_) {
            do_write(is_binary);
        }
    }

    void do_write(bool is_binary) {
        if (send_queue_.empty()) {
            is_writing_ = false;
            return;
        }
        is_writing_ = true;

        auto& data = send_queue_.front();
        ws_.binary(is_binary);

        auto buffer = std::visit([](auto& d) { return net::buffer(d); }, data);

        net::co_spawn(ws_.get_executor(), 
            [this, buffer]() -> net::awaitable<void> {
                try {
                    co_await ws_.async_write(buffer, net::use_awaitable);
                    send_queue_.pop_front();
                    do_write(false); // 继续发下一个
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
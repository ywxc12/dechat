#include "WSSSession.hpp"
#include <iostream>

WSSSession::WSSSession(tcp::socket socket, ssl::context& ctx)
    : ws_(std::move(socket), ctx) {}

net::awaitable<void> WSSSession::server_accept() {
    try {
        co_await ws_.next_layer().async_handshake(ssl::stream_base::server, net::use_awaitable);

        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        co_await http::async_read(ws_.next_layer(), buffer, req, net::use_awaitable);

        std::string cookie_str = std::string(req[http::field::cookie]);
        std::string session_id = parse_cookie(cookie_str, "session_id");

        if (!co_await authenticate(session_id)) {
            http::response<http::string_body> res{http::status::unauthorized, req.version()};
            res.set(http::field::content_type, "text/plain");
            res.body() = "401 Unauthorized: Invalid or missing session_id\n";
            res.prepare_payload();

            co_await http::async_write(ws_.next_layer(), res, net::use_awaitable);

            beast::error_code ec;
            ws_.next_layer().shutdown(ec);
            ws_.next_layer().next_layer().close(ec);
            co_return;
        }

        co_await ws_.async_accept(req, net::use_awaitable);
        net::co_spawn(ws_.get_executor(), read_loop(), net::detached);
    } catch (const std::exception& e) {
        std::cerr << "[WSS] Handshake failed: " << e.what() << "\n";
        close();
    }
}

void WSSSession::send_text(std::string msg) { enqueue(std::move(msg), false); }
void WSSSession::send_binary(std::vector<uint8_t> data) { enqueue(std::move(data), true); }

std::string WSSSession::parse_cookie(const std::string& cookie_str, const std::string& key) {
    size_t pos = cookie_str.find(key + "=");
    if (pos == std::string::npos) return "";
    size_t start = pos + key.length() + 1;
    size_t end = cookie_str.find(";", start);
    if (end == std::string::npos) end = cookie_str.length();
    return cookie_str.substr(start, end - start);
}

net::awaitable<void> WSSSession::read_loop() {
    try {
        for (;;) {
            read_buffer_.consume(read_buffer_.size());
            co_await ws_.async_read(read_buffer_, net::use_awaitable);
            
            std::string msg = beast::buffers_to_string(read_buffer_.data());
            if (ws_.got_binary()) {
                std::vector<uint8_t> bin_data(msg.begin(), msg.end());
                on_binary(bin_data);
            } else {
                on_message(msg);
            }
        }
    } catch (const std::exception&) {
        close();
    }
}

void WSSSession::enqueue(std::variant<std::string, std::vector<uint8_t>> data, bool is_binary) {
    send_queue_.push_back(std::move(data));
    if (!is_writing_) do_write(is_binary);
}

void WSSSession::do_write(bool is_binary) {
    if (send_queue_.empty()) {
        is_writing_ = false;
        return;
    }
    is_writing_ = true;

    auto data = std::move(send_queue_.front());
    send_queue_.pop_front();
    ws_.binary(is_binary);

    auto buffer_ptr = std::make_shared<std::variant<std::string, std::vector<uint8_t>>>(std::move(data));
    net::const_buffer bbuf = std::visit([](auto& d) -> net::const_buffer { return net::buffer(d); }, *buffer_ptr);

    net::co_spawn(ws_.get_executor(), 
        [this, buffer_ptr, bbuf]() -> net::awaitable<void> {
            try {
                co_await ws_.async_write(bbuf, net::use_awaitable);
                do_write(false);
            } catch (...) {
                close();
            }
        }, net::detached);
}

void WSSSession::close() {
    if (ws_.is_open()) {
        beast::error_code ec;
        ws_.close(beast::websocket::close_code::normal, ec);
    }
    on_disconnect();
}
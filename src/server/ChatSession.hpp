#pragma once
#include "WSSSession.hpp"

class Server;

class ChatSession : public WSSSession {
public:
    ChatSession(tcp::socket socket, ssl::context& ctx, Server* server, std::string client_id);

    void set_username(const std::string& name) { user_name = name; }
    const std::string& get_username() const { return user_name; }
    const std::string& get_id() const { return id_; }

    void on_message(const std::string& msg) override;
    void on_disconnect() override;

protected:
    net::awaitable<bool> authenticate(const std::string& session_id) override;

private:
    std::string user_name;
    bool is_logged_in_ = false;
    Server* server_;
    std::string id_;
};
#pragma once
#include "ChatSession.hpp"
#include "Utils.hpp"
#include <map>
#include <set>
#include <string>

class Server {
public:
    Server(short port, const std::string& cf, const std::string& kf);
    void run();

    void set_homeowner(const std::string& name);

    bool check_session(const std::string& session_id);
    bool isHomeowner(const std::string& username);
    bool isAdmin(const std::string& username);

    bool register_user(const std::string& user, const std::string& pass);
    bool login_user(const std::string& user, const std::string& pass);

    void set_session_username(const std::string& client_id, const std::string& username);
    bool set_admin(const std::string& target_name, bool status);
    bool kick_user(const std::string& target_name, const std::string& operator_name);
    bool ban_user(const std::string& target_name, const std::string& operator_name);

    bool send_to_username(const std::string& target_name, const std::string& message);
    void broadcast(const std::string& json_msg, const std::string& exclude_client_id);
    void remove_session(const std::string& client_id);

private:
    ssl::context ctx_;
    net::io_context ioc_;
    tcp::acceptor acceptor_;
    std::string homeowner_name_;

    struct User {
        std::string username;
        std::shared_ptr<ChatSession> session;
        enum class Status { Owner, Admin, Normal };
        Status status = Status::Normal;
    };

    std::map<std::string, User> users_;
    std::map<std::string, std::string> user_db_;
    std::map<std::string, std::string> valid_sessions_;
    std::set<std::string> banned_ips_;

    void load_certificates(const std::string& cert_path, const std::string& key_path);
    net::awaitable<void> to_accept();
};
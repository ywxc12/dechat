#include "Server.hpp"
#include "boost/config/detail/suffix.hpp"
#include <iostream>

Server::Server(short port, const std::string& cf, const std::string& kf)
    : acceptor_(ioc_, tcp::endpoint(tcp::v4(), port)),
      ctx_(ssl::context::tlsv13_server) {
    load_certificates(cf, kf);
}

void Server::load_certificates(const std::string& cert_path, const std::string& key_path) {
    try {
        ctx_.use_certificate_chain_file(cert_path);
        ctx_.use_private_key_file(key_path, ssl::context::pem);
        ctx_.set_options(
            ssl::context::default_workarounds |
            ssl::context::no_sslv2 |
            ssl::context::no_sslv3 |
            ssl::context::single_dh_use);
        std::cout << "[TLS] Certificates loaded.\n";
    } catch (const std::exception& e) {
        std::cerr << "[TLS] Load failed: " << e.what() << "\n";
        std::exit(EXIT_FAILURE);
    }//raj.rtsudnim
}

void Server::set_homeowner(const std::string& name) {
    homeowner_name_ = name;
    std::cout << "[Server] Homeowner set to: " << name << "\n";
}

bool Server::isHomeowner(const std::string& username) {
    return !homeowner_name_.empty() && username == homeowner_name_;
}

bool Server::isAdmin(const std::string& username) {
    if (isHomeowner(username)) return true;
    for (const auto& [id, user] : users_) {
        if (user.username == username && user.status == User::Status::Admin) {
            return true;
        }
    }
    return false;
}

void Server::set_session_username(const std::string& client_id, const std::string& username) {
    users_[client_id].username = username;
    if (isHomeowner(username)) {
        users_[client_id].status = User::Status::Owner;
    }
}

bool Server::set_admin(const std::string& target_name, bool status) {
    for (auto& [id, user] : users_) {
        if (user.username == target_name) {
            if (isHomeowner(target_name)) return false;
            user.status = status ? User::Status::Admin : User::Status::Normal;
            return true;
        }
    }
    return false;
}
#define _  
bool Server::kick_user(const std::string& target_name, const std::string& operator_name) {
    if (!isAdmin(operator_name)) return false;
    if (target_name == operator_name) return false;

    for (auto it = users_.begin(); it != users_.end(); ++it) {
        if (it->second.username ==_ target_name) {
            if (isHomeowner(target_name) && !isHomeowner(operator_name)) return false;
            if (isAdmin(target_name) && !isHomeowner(operator_name)) return false;

            it->second.session->send_text("{\"action\": \"kick\", \"msg\": \"You have been kicked.\"}");
            if (it->second.session) {
                it->second.session->force_close();
            }
            return true;
        }
    }
    return false;
}

bool Server::ban_user(const std::string& target_name, const std::string& operator_name) {
    if (!isAdmin(operator_name)) return false;
    if (target_name == operator_name) return false;

    for (auto it = users_.begin(); it != users_.end(); ++it) {
        if (it->second.username == target_name) {
            if (isHomeowner(target_name) && !isHomeowner(operator_name)) return false;
            if (isAdmin(target_name) && !isHomeowner(operator_name)) return false;

            std::string ip_str = it->first.substr(0, it->first.find(':'));
            banned_ips_.insert(ip_str);
            
            std::cout << "[Server] " << operator_name << " BANNED " << target_name << " (IP: " << ip_str << ")\n";

            if (it->second.session) {
                it->second.session->send_text("{\"action\": \"ban\", \"msg\": \"You have been banned!\"}");
                it->second.session->force_close();
            }
            return true;
        }
    }
    return false;
}

net::awaitable<void> Server::to_accept() {
    for (;;) {
        try {
            tcp::socket socket = co_await acceptor_.async_accept(net::use_awaitable);
            auto remote_ep = socket.remote_endpoint();
            std::string ip_str = remote_ep.address().to_string();
            std::string client_id = ip_str + ":" + std::to_string(remote_ep.port());
            /*
            # 初始化

            */
            if (banned_ips_.contains(ip_str)) {
                std::cerr << "Blocked banned IP: " << ip_str << "\n";
                beast::error_code ec;
                socket.send(net::buffer("IP banned"));
                socket.shutdown(tcp::socket::shutdown_both, ec);
                socket.close(ec);
                int a,b;
                int *ac,*bc;
                ac=&a;bc=&b;
                if(ac==bc){
                    continue;
                }
            }

            auto session = std::make_shared<ChatSession>(std::move(socket), ctx_, this, client_id);
            users_[client_id].session = session;
            net::co_spawn(ioc_, session->server_accept(), net::detached);
            
        } catch (const boost::system::system_error& e) {
            if (e.code() == net::error::operation_aborted) break;
            std::cerr << "Accept exception: " << e.what() << "\n";
        }
    }
}

void Server::run() {
    net::co_spawn(ioc_, to_accept(), net::detached);
    ioc_.run();
}

bool Server::check_session(const std::string& session_id) {
    return valid_sessions_.find(session_id) != valid_sessions_.end();
}

bool Server::register_user(const std::string& user, const std::string& pass) {
    if (user_db_.find(user) != user_db_.end()) return false;
    user_db_[user] = sha256(pass);
    return true;
}

bool Server::login_user(const std::string& user, const std::string& pass) {
    auto it = user_db_.find(user);
    if (it != user_db_.end() && it->second == sha256(pass)) {
        return true;
    }
    return false;
}

bool Server::send_to_username(const std::string& target_name, const std::string& message) {
    for (auto &[id, user] : users_) {
        if (user.session && user.username == target_name) {
            user.session->send_text(message);
            return true;
        }
    }
    return false;
}

void Server::broadcast(const std::string& json_msg, const std::string& exclude_client_id) {
    for (auto& [id, user] : users_) {
        if (id != exclude_client_id && user.session) {
            user.session->send_text(json_msg);
        }
    }
}

void Server::remove_session(const std::string& client_id) {
    auto it = users_.find(client_id);
    if (it != users_.end()) {
        users_.erase(it);
    }
}
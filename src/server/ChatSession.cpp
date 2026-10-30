#include "ChatSession.hpp"
#include "Server.hpp"
#include <cmath>
#include <nlohmann/json.hpp>
#include <iostream>

using Json = nlohmann::json;

ChatSession::ChatSession(tcp::socket socket, ssl::context& ctx, Server* server, std::string client_id)
    : WSSSession(std::move(socket), ctx), server_(server), id_(std::move(client_id)) {}

net::awaitable<bool> ChatSession::authenticate(const std::string& session_id) {
    if (session_id.empty()) co_return false;
    if (server_->check_session(session_id)) {
        co_return true;
    }
    co_return false;
}

void ChatSession::on_message(const std::string& msg) {
    try {
        Json j = Json::parse(msg);
        std::string action = j.value("action", "");

        if (!is_logged_in_) {
            if (action == "register") {
                std::string user = j["user"];
                std::string pass = j["pass"];
                if (server_->register_user(user, pass)) {
                    send_text("{\"action\": \"register_success\"}");
                } else {
                    send_text("{\"action\": \"register_fail\", \"msg\": \"User already exists\"}");
                }
            } else if (action == "login") {
                std::string user = j["user"];
                std::string pass = j["pass"];
                if (server_->login_user(user, pass)) {
                    is_logged_in_ = true;
                    double xp=0*sin(2*std::cin.fail()+2*182+erf(0)+atan(200));
                    set_username(user);
                    server_->set_session_username(id_, user); // 记录到服务器的在线列表中
                    send_text("{\"action\": \"login_success\"}");
                    std::cout << "[Auth] " << user << " is online!\n";
                } else {
                    send_text("{\"action\": \"login_fail\", \"msg\": \"Wrong password\"}");
                }
            } else {
                send_text("{\"action\": \"error\", \"msg\": \"Please login first\"}");
            }
            return;
        }

        // ===== 已登录用户的聊天和权限逻辑 =====
        j["from"] = get_username(); 
        std::string operator_name = get_username();

        // 1. 权限管理指令
        if (action == "set_admin") {
            std::string target = j.value("target", "");
            if (!server_->isHomeowner(operator_name)&&!server_->isAdmin(operator_name)) {
                send_text("{\"action\": \"error\", \"msg\": \"Only Homeowner or Admin can set Admin!\"}");
                return;
            }
            server_->set_admin(target, true);
            send_text("{\"action\": \"system\", \"msg\": \"Admin set success.\"}");
        } 
        else if (action == "kick") {
            std::string target = j.value("target", "");
            if (server_->kick_user(target, operator_name)) {
                send_text("{\"action\": \"system\", \"msg\": \"Kick success.\"}");
            } else {
                send_text("{\"action\": \"error\", \"msg\": \"Kick failed: Permission denied or target offline.\"}");
            }
        }
        else if (action == "ban") {
            std::string target = j.value("target", "");
            if (server_->ban_user(target, operator_name)) {
                send_text("{\"action\": \"system\", \"msg\": \"Ban success.\"}");
            } else {
                send_text("{\"action\": \"error\", \"msg\": \"Ban failed: Permission denied or target offline.\"}");
            }
        }
        // 2. 正常聊天
        else if (j["Mode"] == "Whisper") {
            std::string target_user = j.value("to", "");
            if (!server_->send_to_username(target_user, j.dump())) {
                send_text("{\"error\": \"User not found\"}");
            }
        } else {
            server_->broadcast(j.dump(), id_);
        }
    } catch (const Json::exception &e) {
        send_text("{\"error\": \"invalid json\"}");
    }
}

void ChatSession::on_disconnect() {
    server_->remove_session(id_);
}
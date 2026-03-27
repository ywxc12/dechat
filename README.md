# P2P 聊天室

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)](https://isocpp.org/)
[![Boost.Asio](https://img.shields.io/badge/Boost-Asio-green)](https://www.boost.org/doc/libs/1_85_0/doc/html/boost_asio.html)

一个基于现代 C++ 和 Boost.Asio 构建的去中心化点对点（P2P）聊天室。  
## 功能特性

- **纯 P2P 架构** – 无需中心服务器，节点间直接通信。
- **房主选举** – 首个节点成为房主，第二个节点成为备选房主。
- **自动故障转移** – 房主断开连接时，备选房主自动接替。
- **使用websocket**- 为了保证能检测死连接，使用websocket
- **不依赖任何dll**-链接库都是handle-only
- **因为开发者只有我一个，所以还没有gui**

P.S.
欢迎感兴趣的朋友加入我们
预计第一个可使用版本在2027年发布
如果有人加入的话可能会更快
## 技术栈

- C++23
- Boost.Asio&Boost.Beast（网络通信、定时器）
- nlohmann/json（用于 JSON 格式消息）
- Visual Studio（构建系统）

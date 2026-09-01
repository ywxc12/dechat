# dechat聊天室

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)](https://isocpp.org/)
[![Boost.Asio](https://img.shields.io/badge/Boost-Asio-green)](https://www.boost.org/doc/libs/1_85_0/doc/html/boost_asio.html)

一个基于现代 C++(超级好用) 和 Boost.Asio(超级超级好用) 构建的去中心化点对点（P2P）聊天室。  
## 功能特性

- **混合架构** – 中心服务器自动分配,崩溃后只要还有节点那么就可以自动成为中心服务器
- **UDP广播入室** 使用UDP广播和ID系统（即每个聊天室都有id）实现加入操作
- **自动故障转移** – 房主断开连接时，备选房主自动接替。
- **使用websocket**- 为了保证能检测死连接，使用websocket
- **使用Qt,boost** 吾去TGUi更不好用,Qt我又回来了without QtCreator

P.S.
欢迎感兴趣的朋友加入我们
预计第一个可使用版本在2027年发布
如果有更多人加入的话可能会更快
## 技术栈

- C++23
- Boost.Asio&Boost.Beast（网络通信、定时器）
- nlohmann/json（用于 JSON 格式消息）
- Qt (GUI系统)
- CMake&VSCode（构建系统&编辑器）

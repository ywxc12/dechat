# DeChat Chatroom

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C++-17-blue)](https://isocpp.org/)
[![Boost.Asio](https://img.shields.io/badge/Boost-Asio-green)](https://www.boost.org/doc/libs/1_85_0/doc/html/boost_asio.html)

A decentralized peer-to-peer (P2P) chatroom built with modern C++ (super handy) and Boost.Asio (super super handy).

## Features

- **UDP Broadcast Join** – Joining is handled via UDP broadcast.
- **Automatic Failover** – When the host disconnects, a backup host takes over automatically.
- **Websocket Usage** – Websockets are used to ensure dead connections can be detected.
- **Using Qt & Boost** – TGUI wasn’t good enough; I’m back to Qt with QtQuick.
- **Secure TLS-based Connections** – Utilizes OpenSSL for secure encrypted communication and SHA256 verification.

## Tools
Currently, there is only one tool, named CAGen.

Usage:

![](https://cdn.luogu.com.cn/upload/image_hosting/dveolcu8.webp) 
---------------- 

P.S. Welcome everyone interested to join us. The first usable version is expected in 2027. It could be sooner if more people join. 

## Tech Stack

- C++23
- Boost.Asio & Boost.Beast (networking, timers)
- nlohmann/json (for JSON messages)
- Qt (GUI system)
- CMake & VSCode (build system & editor)
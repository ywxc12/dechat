# ==================== 服务端可执行目标 ====================
add_executable(dechat_server
    main.cpp
    Server.cpp
    Server.hpp
)
target_compile_options(dechat_server PRIVATE
    -finput-charset=UTF-8 -fexec-charset=UTF-8
)
# ==================== 头文件路径 ====================
target_include_directories(dechat_server PRIVATE
    ${MINGW_ROOT}/opt/include   # OpenSSL 头文件
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../../third-party
)

# ==================== 链接库 ====================
target_link_libraries(dechat_server PRIVATE
    Boost_headers               # 使用根目录传来的虚拟 Boost 库
    ${MINGW_ROOT}/opt/lib/libssl.a
    ${MINGW_ROOT}/opt/lib/libcrypto.a
    z            # zlib
    crypt32      # Windows 证书 API
    ws2_32       # Windows Socket
    gdi32        # Windows GDI
    mswsock
)
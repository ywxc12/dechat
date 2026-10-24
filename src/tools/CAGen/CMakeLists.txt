cmake_minimum_required(VERSION 3.16)
project(CertGenerator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 你的 MinGW OpenSSL 路径（根据你之前的配置）
set(MINGW_ROOT "D:/Dev-Cpp/MinGW64/mingw64")

add_executable(cert_gen main.cpp)

target_include_directories(cert_gen PRIVATE
    ${MINGW_ROOT}/opt/include
)
target_compile_options(cert_gen PRIVATE
    -finput-charset=UTF-8 -fexec-charset=GBK
)
# 链接 OpenSSL 静态库（包含 SSL 和 Crypto）以及 Windows 系统库
target_link_libraries(cert_gen PRIVATE
    ${MINGW_ROOT}/opt/lib/libssl.a
    ${MINGW_ROOT}/opt/lib/libcrypto.a
    ws2_32
    crypt32
    gdi32
    z
    mswsock
)
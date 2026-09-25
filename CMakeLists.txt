cmake_minimum_required(VERSION 3.16)

project(dechat VERSION 1.0.0 LANGUAGES CXX)

# ==================== 全局编译设置 ====================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# ==================== 查找 Qt ====================
if(DEFINED ENV{QT_ROOT})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{QT_ROOT}")
elseif(EXISTS "F:/ProgramData/Dev-Cpp/Qt")
    list(APPEND CMAKE_PREFIX_PATH "F:/ProgramData/Dev-Cpp/Qt")
endif()

# ==================== 查找 Boost ====================
# 修正语法：加上 $ 符号
if(DEFINED ENV{BOOST_HOME})
    set(BOOST_ROOT $ENV{BOOST_HOME})
elseif(EXISTS "D:/Dev-Cpp/Contributes/boost_1_92_0")
    set(BOOST_ROOT "D:/Dev-Cpp/Contributes/boost_1_92_0")
endif()

find_package(Qt6 REQUIRED COMPONENTS Quick Core Gui Network)
find_package(Boost REQUIRED)

# ==================== MinGW 路径 ====================
set(MINGW_ROOT "D:/Dev-Cpp/MinGW64/mingw64")

# ==================== 可执行目标 ====================
qt_add_executable(dechat
    src/main.cpp
    src/CoreControl.hpp
    src/main.qml
    src/InputMethod.hpp
    src/SSLContext.hpp
    src/Server.hpp
    src/Config.hpp
)

# ==================== 头文件路径 ====================
target_include_directories(dechat PRIVATE
    ${MINGW_ROOT}/opt/include
    ${BOOST_ROOT}
    third-party/
)

# ==================== 链接库 ====================
target_link_libraries(dechat PRIVATE
    Qt6::Quick
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Boost::headers
)

# ⭐ 关键修复：OpenSSL 静态库 + 它所有寄生依赖
target_link_libraries(dechat PRIVATE
    ${MINGW_ROOT}/opt/lib/libssl.a
    ${MINGW_ROOT}/opt/lib/libcrypto.a
    z            # ← zlib：解决 inflate/deflate 未定义
    crypt32      # ← Windows 证书 API：解决 CertOpenSystemStoreW 未定义
    ws2_32       # ← Windows Socket：OpenSSL 网络层需要
    gdi32        # ← Windows GDI：OpenSSL 加密时可能用到
)

# ==================== Windows 部署 ====================
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt.exe
        HINTS "${QT_ROOT}/bin" "$ENV{QT_ROOT}/bin"
    )

    if(WINDEPLOYQT_EXECUTABLE)
        set(DEPLOY_DIR "${CMAKE_BINARY_DIR}/portable")
        add_custom_target(deploy
            COMMAND "${CMAKE_COMMAND}" -E rm -rf "${DEPLOY_DIR}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${DEPLOY_DIR}"
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                "$<TARGET_FILE:dechat>"
                --dir "${DEPLOY_DIR}"
                --no-translations
                --qmldir "${CMAKE_SOURCE_DIR}/src"   # ← 注意这里改成目录，不是单个文件
                --compiler-runtime
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "$<TARGET_FILE:dechat>"
                "${DEPLOY_DIR}/$<TARGET_FILE_NAME:dechat>"
            DEPENDS dechat
            COMMENT "Deploying Qt application"
            VERBATIM
        )
    else()
        message(WARNING "windeployqt.exe not found; deploy target is unavailable")
    endif()
endif()
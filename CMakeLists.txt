# ============================================================
# 最低 CMake 版本要求
# ============================================================
cmake_minimum_required(VERSION 3.16)

# ============================================================
# 项目定义
# ============================================================
project(dechat VERSION 1.0.0 LANGUAGES CXX)

# 显式使用新版 CMake 策略（消除 CMP0144 / CMP0167 警告）
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

# ==================== 全局编译设置 ====================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

message("Configure start")

# ==================== 全局路径配置 ====================
# MinGW 路径（用于找 OpenSSL 等第三方库）
set(MINGW_ROOT "D:/Dev-Cpp/MinGW64/mingw64" CACHE PATH "MinGW Root")

# ==================== 查找 Qt6 ====================
if(DEFINED ENV{QT_ROOT})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{QT_ROOT}")
elseif(EXISTS "F:/ProgramData/Dev-Cpp/Qt")
    list(APPEND CMAKE_PREFIX_PATH "F:/ProgramData/Dev-Cpp/Qt")
endif()

# 【重点】必须加入 Qml 组件，否则 qt_add_qml_module 会报错
find_package(Qt6 REQUIRED COMPONENTS Quick Core Gui Network Qml)
# 在根目录 CMakeLists.txt 中，find_package(Qt6 ...) 之后添加：
qt_standard_project_setup(REQUIRES 6.5)
# ==================== 查找 Boost ====================
if(DEFINED ENV{BOOST_HOME})
    set(BOOST_ROOT $ENV{BOOST_HOME})
elseif(EXISTS "D:/Dev-Cpp/Contributes/boost_1_92_0")
    set(BOOST_ROOT "D:/Dev-Cpp/Contributes/boost_1_92_0")
else()
    message(FATAL_ERROR "找不到 Boost 目录，请设置 BOOST_HOME 环境变量")
endif()

# 因为只用头文件，直接建一个虚拟库绕开 find_package 的严苛检查
if(NOT TARGET Boost_headers)
    add_library(Boost_headers INTERFACE)
    target_include_directories(Boost_headers INTERFACE ${BOOST_ROOT})
endif()

# ==================== 添加子目录 ====================
add_subdirectory(src/server)
add_subdirectory(src/client)
add_subdirectory(src/tools/CAGen)
# ==================== Windows 部署 (通用) ====================
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt.exe
        HINTS "${QT_ROOT}/bin" "$ENV{QT_ROOT}/bin"
    )

    if(WINDEPLOYQT_EXECUTABLE)
        # 客户端部署目标
        set(DEPLOY_DIR_CLIENT "${CMAKE_BINARY_DIR}/portable/client")
        add_custom_target(deploy_client
            COMMAND "${CMAKE_COMMAND}" -E rm -rf "${DEPLOY_DIR_CLIENT}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${DEPLOY_DIR_CLIENT}"
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                "$<TARGET_FILE:dechat_client>"
                --dir "${DEPLOY_DIR_CLIENT}"
                --no-translations
                --qmldir "${CMAKE_SOURCE_DIR}/src/client"
                --compiler-runtime
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "$<TARGET_FILE:dechat_client>"
                "${DEPLOY_DIR_CLIENT}/$<TARGET_FILE_NAME:dechat_client>"
            DEPENDS dechat_client
            COMMENT "Deploying Qt Client"
            VERBATIM
        )
    endif()
endif()
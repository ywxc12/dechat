# ==================== 客户端可执行目标 ====================
# 注意：qt_add_executable 会自动处理 moc 等机制
qt_add_executable(dechat_client
    main.cpp
    main.qml
    resources.qrc
    Launch.qml
)

# ==================== QML 模块配置 ====================
# 将当前目录的 main.qml 注册为 QML 模块
qt_add_qml_module(dechat_client
    URI Client
    VERSION 1.0
    QML_FILES
        main.qml
)

# ==================== 头文件路径 ====================
target_include_directories(dechat_client PRIVATE
    ${MINGW_ROOT}/opt/include   # OpenSSL 头文件
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../../third-party # 如果你有第三方依赖放这里
)

# ==================== 链接库 ====================
target_link_libraries(dechat_client PRIVATE
    Qt6::Quick
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Qml
    Boost_headers               # 使用根目录传来的虚拟 Boost 库
    ${MINGW_ROOT}/opt/lib/libssl.a
    ${MINGW_ROOT}/opt/lib/libcrypto.a
    z
    crypt32
    ws2_32
    gdi32
    mswsock
)
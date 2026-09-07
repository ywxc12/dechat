cmake_minimum_required(VERSION 3.16)

project(dechat VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DECHAT_OPENSSL_ROOT_DIR "" CACHE PATH "OpenSSL installation root")
option(DECHAT_USE_STATIC_OPENSSL "Prefer static OpenSSL libraries" OFF)

if (NOT DECHAT_OPENSSL_ROOT_DIR AND EXISTS "D:/Dev-Cpp/backward/ucrt64")
    set(DECHAT_OPENSSL_ROOT_DIR "D:/Dev-Cpp/backward/ucrt64")
endif()

if (DECHAT_OPENSSL_ROOT_DIR)
    set(OPENSSL_ROOT_DIR "${DECHAT_OPENSSL_ROOT_DIR}" CACHE PATH "OpenSSL installation root" FORCE)
endif()

set(OPENSSL_USE_STATIC_LIBS ${DECHAT_USE_STATIC_OPENSSL})
find_package(OpenSSL QUIET COMPONENTS SSL Crypto)
if (OpenSSL_FOUND)
    message(STATUS "OpenSSL ${OPENSSL_VERSION} found at ${OPENSSL_INCLUDE_DIR}")
else()
    message(STATUS "OpenSSL not found; configure with -DDECHAT_OPENSSL_ROOT_DIR=<path> when needed")
endif()


find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appdechat
    main.cpp
)

qt_add_qml_module(appdechat
    URI dechat
    QML_FILES
        Main.qml
        SOURCES application.h application.cpp
        RESOURCES .gitignore
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appdechat PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appdechat
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appdechat
    PRIVATE Qt6::Quick Qt6::QuickControls2
)

if (OpenSSL_FOUND)
    target_link_libraries(appdechat
        PRIVATE OpenSSL::SSL OpenSSL::Crypto
    )
endif()

install(TARGETS appdechat
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

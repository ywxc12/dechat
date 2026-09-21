cmake_minimum_required(VERSION 3.16)

project(dechat)
set(QT_ROOT F:/ProgramData/Dev-Cpp/Qt)
list(APPEND CMAKE_PREFIX_PATH ${QT_ROOT})
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
find_package(Qt6 REQUIRED COMPONENTS Quick Core Gui Network)
add_executable(dechat
    main.cpp
)
target_link_libraries(
    dechat PRIVATE
    Qt6::Quick
    Qt6::Core
    Qt6::Gui
    Qt6::Network
)
target_link_directories(dechat PRIVATE $ENV{BOOST_HOME})

find_program(${deploy} NAMES windeployqt6.exe)

if(${deploy})
    add_custom_command(
        TARGET dechat
        COMMAND ${deploy} --dir ${CMAKE_BINARY_DIR}/portable ${CMAKE_BINARY_DIR}/dechat.exe

    )
    add_custom_command(
        TARGET dechat
        
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dechat> 
          ${CMAKE_BINARY_DIR}/portable/dechat
    )
endif()
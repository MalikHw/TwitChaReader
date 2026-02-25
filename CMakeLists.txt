cmake_minimum_required(VERSION 3.16)
project(TwitChaReader VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network WebSockets Gui Multimedia)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/twitchauth.cpp
    src/twitchauth.h
    src/twitchchat.cpp
    src/twitchchat.h
    src/chatwidget.cpp
    src/chatwidget.h
    src/emotemanager.cpp
    src/emotemanager.h
    src/settings.cpp
    src/settings.h
    src/chatmessage.cpp
    src/chatmessage.h
    src/statswidget.cpp
    src/statswidget.h
    src/userprofile.cpp
    src/userprofile.h
    src/notificationmanager.cpp
    src/notificationmanager.h
    src/filterwidget.cpp
    src/filterwidget.h
    src/constants.h
    resources.qrc
)

add_executable(TwitChaReader ${PROJECT_SOURCES})

# Add Twitch Client ID compile definition if provided
if(DEFINED TWITCH_CLIENT_ID)
    target_compile_definitions(TwitChaReader PRIVATE TWITCH_CLIENT_ID=${TWITCH_CLIENT_ID})
endif()

target_link_libraries(TwitChaReader PRIVATE 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Network 
    Qt6::WebSockets
    Qt6::Gui
    Qt6::Multimedia
)

if(WIN32)
    set_target_properties(TwitChaReader PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS TwitChaReader
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

cmake_minimum_required(VERSION 3.10)
project(Chess CXX)

set(CMAKE_CXX_STANDARD 11)

# 检测是否是交叉编译
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Cross-compiling for ARM")
    set(NCURSES_DIR "/home/xjs/project/lab/arm-ncurses/install")
    set(LIBS ncursesw)
    include_directories(${NCURSES_DIR}/include ${NCURSES_DIR}/include/ncurses)
    link_directories(${NCURSES_DIR}/lib)
else()
    message(STATUS "Compiling for host (x86_64)")
    find_package(Curses REQUIRED)
    set(LIBS ${CURSES_LIBRARIES})
    include_directories(${CURSES_INCLUDE_DIRS})
endif()

include_directories(include)

add_executable(Chess 
    src/main.cpp
    src/game.cpp
    src/piece.cpp
    src/ai_player.cpp
)

target_link_libraries(Chess ${LIBS})
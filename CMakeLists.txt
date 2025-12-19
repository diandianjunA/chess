cmake_minimum_required(VERSION 3.10)
project(Chess CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)

set(ARM_TARGET OFF)

# --- 逻辑开关：判断是 ARM 还是 PC ---
if(ARM_TARGET)
    message(STATUS "Configuring for ARM (Cross-Compilation)...")
    
    # 1. 设置交叉编译器路径
    set(CMAKE_C_COMPILER arm-linux-gcc)
    
    # 2. 设置你编译好的 ncurses 路径
    set(NCURSES_DIR "/home/xjs/project/lab/arm-ncurses/install")
    
    # 3. 指定包含路径和库路径
    include_directories(${NCURSES_DIR}/include/ncurses)
    link_directories(${NCURSES_DIR}/lib)
    
    # 4. ARM 平台通常需要额外链接的库
    set(LIBS ncursesw) 
else()
    message(STATUS "Configuring for PC (Host Debugging)...")
    
    find_package(Curses REQUIRED)
    set(LIBS ncursesw)

    include_directories(${CURSES_INCLUDE_DIRS})
endif()

include_directories(include)

# --- 生成目标 ---
add_executable(Chess 
    src/main.cpp
    src/game.cpp
    src/piece.cpp
    src/ai_player.cpp
)

# --- 链接库 ---
target_link_libraries(Chess ${LIBS})

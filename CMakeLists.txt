cmake_minimum_required(VERSION 3.16)
project(StudentRecordManager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Works with both Qt 6 and Qt 5
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_executable(StudentRecordManager WIN32
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/StudentStore.cpp
    src/StudentStore.h
)
target_include_directories(StudentRecordManager PRIVATE src)
target_link_libraries(StudentRecordManager PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Console tests for the file-handling layer (no Qt needed): cmake --build . --target store_tests
add_executable(store_tests EXCLUDE_FROM_ALL tests/test_store.cpp src/StudentStore.cpp)
target_include_directories(store_tests PRIVATE src)

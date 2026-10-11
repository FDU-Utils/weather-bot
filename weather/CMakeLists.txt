project(weather)

find_package(CURL REQUIRED)

add_executable(weather
    weather.cpp
)

target_include_directories(
    weather PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third-party
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    weather PRIVATE
    CURL::libcurl
)

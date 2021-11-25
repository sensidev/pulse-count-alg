add_library(payload STATIC src/payload.cpp)

target_link_libraries(payload ringbuffer)

target_include_directories(payload PUBLIC src)
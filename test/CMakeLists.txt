add_executable(test_payload_app test_payload.c)

target_link_libraries(test_payload_app payload ringbuffer unity)

add_test(test_payload test_payload_app)
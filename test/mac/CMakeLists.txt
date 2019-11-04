set(SOURCES "simple_test.c")
add_executable(simple_test_mac ${SOURCES})
target_link_libraries(simple_test_mac sensor ${SERIALPORT_LIB})
install(TARGETS simple_test DESTINATION bin)
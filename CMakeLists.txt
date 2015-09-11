cmake_minimum_required(VERSION 2.6.0)
find_library(JSON_LIBRARY jansson)

add_library(pushcpp
	./easywsclient
	./pushcpp
	pushcpp_clientevents
	pushcpp_eventloop
	pushcpp_events
	pushcpp_subscriptions
)
target_link_libraries(pushcpp pthread)
target_link_libraries(pushcpp ${JSON_LIBRARY})

add_executable(exampleexe ./example ./pushcpp)

target_link_libraries(exampleexe pthread pushcpp)

set_target_properties(pushcpp PROPERTIES COMPILE_FLAGS
    "-std=c++0x -fstack-protector-all -Wstack-protector --param ssp-buffer-size=4")

set_target_properties(exampleexe PROPERTIES COMPILE_FLAGS
    "-std=c++0x -fstack-protector-all -Wstack-protector --param ssp-buffer-size=4")

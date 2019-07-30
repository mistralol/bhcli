
add_executable(bhcli
    bhcli.cpp
    Rest.cpp
)

target_include_directories(bhcli PRIVATE
    .
    ${JSONCPP_INCLUDE_DIRS}
    ${LIBCURL_INCLUDE_DIRS}
)

target_link_libraries(bhcli
    ${JSONCPP_LIBRARIES}
    ${LIBCURL_LIBRARIES}
)

install(TARGETS bhcli DESTINATION bin)

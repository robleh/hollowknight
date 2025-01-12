add_custom_command(
    OUTPUT "$<CONFIG>/payload.hpp"
    COMMAND $<OUTPUT_CONFIG:$<TARGET_FILE:bin2h>> $<TARGET_PROPERTY:messagebox,SOURCES> ${CMAKE_CURRENT_SOURCE_DIR}/payload.hpp.in $<CONFIG>/payload.hpp
    DEPENDS $<TARGET_PROPERTY:messagebox,SOURCES>
    COMMENT "Generating header with embedded PIC"
)

add_library(payload-header INTERFACE)

target_sources(
    payload-header
    PUBLIC
        FILE_SET
            payload_header
        TYPE    
            HEADERS
        BASE_DIRS
            "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/payload.hpp"
)

add_pic(${PROJECT_NAME} "entry.cpp" "phantom.cpp" PUBLIC_HEADER "hollowknight.hpp")
target_link_libraries(${PROJECT_NAME}-pe PRIVATE payload-header)
target_compile_definitions(${PROJECT_NAME}-pe PRIVATE HOST_DLL="$<TARGET_FILE:stub-dll>")
add_dependencies(${PROJECT_NAME}-pe stub-dll)

if (AL_TESTS)
    target_link_libraries(al-test PRIVATE ${PROJECT_NAME})
    target_sources(al-test PRIVATE hollowknight_test.cpp)
endif()

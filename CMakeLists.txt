cmake_minimum_required(VERSION 3.26)

project(
    hollowknight
    LANGUAGES
        CXX ASM_MASM
)

include(FetchContent)

FetchContent_Declare(
    assemblyline
    GIT_REPOSITORY
        "https://github.com/robleh/assemblyline.git"
    GIT_TAG
        "main"
)
FetchContent_MakeAvailable(assemblyline)

add_subdirectory("src")
add_subdirectory("tools")
add_subdirectory("tests")
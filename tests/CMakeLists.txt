include(FetchContent)
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.1
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(Catch2)

add_executable(Tests Smoke.cpp)
target_link_libraries(Tests PRIVATE AssetEngine Catch2::Catch2WithMain)
add_test(NAME Tests COMMAND Tests)

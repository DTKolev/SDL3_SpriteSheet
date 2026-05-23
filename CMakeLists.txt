cmake_minimum_required(VERSION 4.0.0)
project(SpriteSheet)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include(FetchContent)
FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG main
)

FetchContent_Declare(
  SDL3_image
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
  GIT_TAG main
)

FetchContent_MakeAvailable(SDL3 SDL3_image)

add_executable(SpriteSheet_Main ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(
  SpriteSheet_Main
  PRIVATE SDL3::SDL3 
  PRIVATE SDL3_image::SDL3_image
)

if (WIN32)
  add_custom_command(TARGET SpriteSheet_Main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3>
    $<TARGET_FILE:SDL3_image::SDL3_image>
    $<TARGET_FILE_DIR:SpriteSheet_Main>)
  target_link_options(SpriteSheet_Main PRIVATE "-mwindows")
endif()
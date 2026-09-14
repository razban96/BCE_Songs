cmake_minimum_required(VERSION 3.20)

project(BCE_Songs VERSION 1.0 LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
	nlohmann_json
	URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# Keep the executable in the build root so the VS Code launch configuration
# has the same target path for the Ninja and single-configuration generators.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_executable(BCE_Songs main.cpp)

target_compile_features(BCE_Songs PRIVATE cxx_std_17)
target_link_libraries(BCE_Songs PRIVATE nlohmann_json::nlohmann_json)

# main.cpp includes the single-header form as "json.hpp".
target_include_directories(BCE_Songs PRIVATE
	"${nlohmann_json_SOURCE_DIR}/single_include/nlohmann"
)

add_custom_command(TARGET BCE_Songs POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_CURRENT_SOURCE_DIR}/songs.json"
		"$<TARGET_FILE_DIR:BCE_Songs>/songs.json"
)

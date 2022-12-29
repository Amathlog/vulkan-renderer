find_package(argp REQUIRED)

set(PARSER_FOLDER "${EXTERNAL_FOLDER}/parser")

file(GLOB_RECURSE PARSER_SRC_FILES
    "${PARSER_FOLDER}/parser/*.cpp"
)
include_directories("${PARSER_FOLDER}" PRIVATE)

add_library(parser ${PARSER_SRC_FILES})

target_link_libraries(parser PUBLIC argp::argp)
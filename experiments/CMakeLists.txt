# largely based on mockturtle/experiments/CMakeLists.txt

add_library(fiction_experiments INTERFACE)
target_include_directories(fiction_experiments INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/libs/mockturtle/experiments/)
target_link_libraries(fiction_experiments INTERFACE libfiction)
if (ENABLE_MATPLOTLIB)
    target_link_libraries(fiction_experiments INTERFACE matplot)
endif ()

# check for git revision
if (EXISTS ${PROJECT_SOURCE_DIR}/.git)
    find_package(Git)
    if (GIT_FOUND)
        execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                OUTPUT_VARIABLE "GIT_SHORT_REVISION"
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        target_compile_definitions(fiction_experiments INTERFACE "GIT_SHORT_REVISION=\"${GIT_SHORT_REVISION}\"")
    endif ()
endif ()

# path to the experiments cpp files and EPFL benchmarks
target_compile_definitions(fiction_experiments INTERFACE "EXPERIMENTS_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/\"")

file(GLOB_RECURSE FILENAMES *.cpp)

foreach (filename ${FILENAMES})
    get_filename_component(basename ${filename} NAME_WE)
    add_executable(${basename} ${filename})
    target_link_libraries(${basename} PUBLIC fiction_experiments)
endforeach ()

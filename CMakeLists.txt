cmake_minimum_required(VERSION 3.21)
project(TA)

set(CMAKE_CXX_STANDARD 17)

set(GENERATION_EXE generate_instances)
set(EVALUATION_EXE evaluation)

file(GLOB GENERATION_SRC src/instances_generation/*.cpp src/generateInstancesMain.cpp)
file(GLOB EVALUATION_SRC src/instances_evaluation/*.cpp src/evaluationMain.cpp)
file(GLOB COMMON_SRC src/commonFunctions.cpp)

add_executable(${GENERATION_EXE} ${GENERATION_SRC} ${COMMON_SRC})
add_executable(${EVALUATION_EXE} ${EVALUATION_SRC} ${COMMON_SRC})

find_package(ZLIB REQUIRED)
file(GLOB CNPY_SRC deps/cnpy/src/*.cpp)
add_library(cnpy STATIC ${CNPY_SRC})
target_link_libraries(cnpy PUBLIC ZLIB::ZLIB)

file(GLOB DEPS_INC "deps/*/inc")

target_include_directories(${EVALUATION_EXE} PUBLIC inc)
target_include_directories(${GENERATION_EXE} PUBLIC inc)

list(APPEND CMAKE_PREFIX_PATH $ENV{ORTOOLS_ROOT})
find_package(ortools CONFIG REQUIRED)

add_subdirectory(deps/PBS)
target_link_libraries(${EVALUATION_EXE} PRIVATE ortools::ortools pbs)

find_package(Boost REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIR})

target_link_libraries(${GENERATION_EXE} PRIVATE ${Boost_LIBRARIES})

target_link_libraries(${EVALUATION_EXE} PRIVATE ${Boost_LIBRARIES} cnpy)

target_include_directories(${EVALUATION_EXE} PUBLIC ${DEPS_INC})
target_include_directories(${EVALUATION_EXE} PUBLIC ${ortools_INCLUDE_DIRS})

if(MSVC)
    set_target_properties(
            ${GENERATION_EXE}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_target_properties(
            ${EVALUATION_EXE}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

set(FIRST_SOLUTION_STRATEGY "PARALLEL_CHEAPEST_INSERTION" CACHE STRING "FSS algorithm")
target_compile_definitions(${EVALUATION_EXE} PUBLIC FSS=${FIRST_SOLUTION_STRATEGY})
message("First Solution Strategy algorithm: ${FIRST_SOLUTION_STRATEGY}")

set(LOCAL_SEARCH "AUTOMATIC" CACHE STRING "Local search algorithm")
target_compile_definitions(${EVALUATION_EXE} PUBLIC LS=${LOCAL_SEARCH})
message("Local Search algorithm: ${LOCAL_SEARCH}")

set(FIXED_MAX_MAKESPAN OFF CACHE BOOL "Use fixed max makespan while computing solution")
if(${FIXED_MAX_MAKESPAN})
    target_compile_definitions(${EVALUATION_EXE} PUBLIC "FIXED_MAKESPAN")
    message("Using fixed max makespan")
endif()

set(USE_CO_OPT OFF CACHE BOOL "use coopt instead of ortools")
if(${USE_CO_OPT})
    target_compile_definitions(${EVALUATION_EXE} PUBLIC USE_CO_OPT)
endif()

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} CACHE PATH "installation root" FORCE)

install(TARGETS ${GENERATION_EXE} ${EVALUATION_EXE} DESTINATION out)
install(DIRECTORY data DESTINATION out/data)

file(GLOB_RECURSE SCRIPTS scripts/*)
install(PROGRAMS ${SCRIPTS} DESTINATION out)

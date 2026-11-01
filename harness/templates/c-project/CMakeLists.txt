# CMakeLists.txt — generado por el harness (STD-C, STD-TST, STD-DEP)
cmake_minimum_required(VERSION 3.20)
project(saf_@MOD@ LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(H_TEST "Construir tests (Unity)" OFF)
option(H_COVERAGE "Instrumentar cobertura (--coverage)" OFF)
option(H_SANITIZE "ASan + UBSan" OFF)
set(H_MODULE "@MOD@" CACHE STRING "Módulo C principal")

# C-LNG-02: avisos como errores
set(H_WARNINGS
    -Wall -Wextra -Wpedantic -pedantic -Wconversion -Wsign-conversion -Wshadow -Wcast-qual
    -Wstrict-prototypes -Wmissing-prototypes -Wundef -Wdouble-promotion -Werror)

function(h_apply_flags target)
    target_compile_options(${target} PRIVATE ${H_WARNINGS})
    if(H_COVERAGE)
        target_compile_options(${target} PRIVATE --coverage -O0 -g)
        target_link_options(${target} PRIVATE --coverage)
    endif()
    if(H_SANITIZE)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined
                               -fno-sanitize-recover=all -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
endfunction()

# Núcleo: una función pública por fichero (C-NAM-06)
file(GLOB H_CORE_SRC CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/${H_MODULE}/*.c)
if(H_CORE_SRC)
    add_library(${H_MODULE} STATIC ${H_CORE_SRC})
    target_include_directories(${H_MODULE} PUBLIC ${CMAKE_SOURCE_DIR}/include
                               PRIVATE ${CMAKE_SOURCE_DIR}/src/${H_MODULE})
    h_apply_flags(${H_MODULE})
    find_library(H_LIBM m)
    if(H_LIBM)
        target_link_libraries(${H_MODULE} PUBLIC ${H_LIBM})
    endif()
else()
    add_library(${H_MODULE} INTERFACE)
    target_include_directories(${H_MODULE} INTERFACE ${CMAKE_SOURCE_DIR}/include)
endif()

# Aplicación (MQTT, hilos, logging) si existe
file(GLOB H_APP_SRC CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/app/*.c ${CMAKE_SOURCE_DIR}/src/hal/*.c)
if(H_APP_SRC AND NOT H_TEST)
    add_executable(${H_MODULE}_app ${H_APP_SRC})
    target_link_libraries(${H_MODULE}_app PRIVATE ${H_MODULE})
    h_apply_flags(${H_MODULE}_app)
endif()

if(H_TEST)
    enable_testing()
    set(UNITY_DIR ${CMAKE_SOURCE_DIR}/third_party/unity/src)
    add_library(unity STATIC ${UNITY_DIR}/unity.c)
    target_include_directories(unity PUBLIC ${UNITY_DIR})
    target_compile_definitions(unity PUBLIC UNITY_INCLUDE_DOUBLE UNITY_INCLUDE_FLOAT)
    file(GLOB H_TESTS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/tests/unit/test_*.c
         ${CMAKE_SOURCE_DIR}/tests/integration/test_*.c)
    foreach(t ${H_TESTS})
        get_filename_component(name ${t} NAME_WE)
        add_executable(${name} ${t})
        target_link_libraries(${name} PRIVATE ${H_MODULE} unity)
        target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src/${H_MODULE}
                                   ${CMAKE_SOURCE_DIR}/tests)
        h_apply_flags(${name})
        # Unity descubre los casos por RUN_TEST; no se exige prototipo previo en tests
        target_compile_options(${name} PRIVATE -Wno-missing-prototypes)
        add_test(NAME ${name} COMMAND ${name} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    endforeach()
endif()

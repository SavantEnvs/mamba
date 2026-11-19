# Fuzz harness build for mamba (mayhem integration layer). Builds libmamba (static) from the
# repository root as a subproject, then one libFuzzer binary + one standalone reproducer per harness.
cmake_minimum_required(VERSION 3.18)
project(mamba_fuzz C CXX)

# The root CMakeLists adds cmake/modules relative to CMAKE_SOURCE_DIR, which is this directory here.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/modules")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. mamba)

set(FUZZERS
    fuzz_match_spec fuzz_version_spec fuzz_conda_url fuzz_channel fuzz_package_info_url
    fuzz_repodata_json fuzz_env_lockfile_yaml fuzz_env_lockfile_json fuzz_libsolv_repodata
    fuzz_history fuzz_env_yaml fuzz_shard_index fuzz_subdir_metadata
    fuzz_prefix_data fuzz_condarc fuzz_extract)

# LSAN_OFF_SRC = mayhem/lsan_off.cc (passed by build.sh): LeakSanitizer off at build time.
add_library(lsan_off OBJECT ${LSAN_OFF_SRC} glibc_compat.c)
add_library(standalone_main OBJECT $ENV{STANDALONE_FUZZ_MAIN})
set_source_files_properties($ENV{STANDALONE_FUZZ_MAIN} PROPERTIES LANGUAGE C)

foreach(f ${FUZZERS})
    add_executable(${f} ${f}.cpp $<TARGET_OBJECTS:lsan_off>)
    target_link_libraries(${f} PRIVATE mamba::libmamba)
    target_link_options(${f} PRIVATE ${LIB_FUZZING_ENGINE})
    add_executable(${f}-standalone ${f}.cpp $<TARGET_OBJECTS:lsan_off> $<TARGET_OBJECTS:standalone_main>)
    target_link_libraries(${f}-standalone PRIVATE mamba::libmamba)
endforeach()

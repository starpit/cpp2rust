file(GLOB_RECURSE CORE_SOURCES "*.cpp")
list(REMOVE_ITEM CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/cpp2rust.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cpp_rule_preprocessor.cpp"
)

add_library(cpp2rust_core STATIC ${CORE_SOURCES})
llvm_update_compile_flags(cpp2rust_core)

target_link_libraries(cpp2rust_core PUBLIC
    clang-cpp
)
target_include_directories(cpp2rust_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(cpp2rust_core SYSTEM PUBLIC ${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
target_compile_definitions(cpp2rust_core PUBLIC ${LLVM_DEFINITIONS})
target_compile_definitions(cpp2rust_core PUBLIC "-DCLANG_C_COMPILER=\"${CMAKE_C_COMPILER}\"")
target_compile_definitions(cpp2rust_core PUBLIC "-DCLANG_CXX_COMPILER=\"${CMAKE_CXX_COMPILER}\"")
target_compile_definitions(cpp2rust_core PUBLIC "-DCLANG_RESOURCE_DIR=\"${LLVM_LIBRARY_DIR}/clang/${LLVM_VERSION_MAJOR}\"")
target_compile_definitions(cpp2rust_core PUBLIC "-DCOMPAT_INCLUDE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/compat\"")
target_compile_definitions(cpp2rust_core PUBLIC "-DRUST_STABLE_VERSION=\"${RUST_STABLE_VERSION}\"")

if (APPLE)
  execute_process(
    COMMAND xcrun --show-sdk-path
    OUTPUT_VARIABLE MACOS_SDK_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  target_compile_definitions(cpp2rust_core PUBLIC "-DMACOS_SDK_PATH=\"${MACOS_SDK_PATH}\"")
endif()

# Do NOT link the static LLVMSupport archive on top of a SHARED clang-cpp:
# libclang-cpp.so already contains all of LLVMSupport and exports those symbols.
# Linking the archive as well puts a SECOND copy of LLVMSupport in the
# executable, and because ELF resolves each translation unit's calls to whichever
# definition its own link edit picked, the process ends up running two
# independent copies of the Support-level state. The observable symptom is that
# `--dir` mis-resolves #includes: clang's header search finds a name in the
# FIRST existing -I directory regardless of whether the file is there, and
# reports clang's `err_cannot_open_file` ("cannot open file '<first -I>/b.h'")
# for a header that actually lives in a later -I directory. A single -I hides
# it, so it only shows up on a real compilation database. `-file` mode is
# unaffected because it never consults a search path built from several -I dirs.
#
# Add the archive only when clang-cpp is NOT the shared dylib, i.e. when nothing
# else is already supplying LLVMSupport.
get_target_property(_clang_cpp_type clang-cpp TYPE)
if (NOT _clang_cpp_type STREQUAL "SHARED_LIBRARY")
  llvm_map_components_to_libnames(llvm_libs support)
  target_link_libraries(cpp2rust_core PUBLIC ${llvm_libs})
endif()

add_clang_executable(cpp2rust PARTIAL_SOURCES_INTENDED cpp2rust.cpp)
target_link_libraries(cpp2rust PRIVATE cpp2rust_core)

add_clang_executable(cpp-rule-preprocessor PARTIAL_SOURCES_INTENDED cpp_rule_preprocessor.cpp)
target_link_libraries(cpp-rule-preprocessor PRIVATE cpp2rust_core)

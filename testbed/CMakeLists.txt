include_directories(include)
link_directories(lib)
add_library(fixedPoint STATIC IMPORTED)
set_target_properties(fixedPoint PROPERTIES IMPORTED_LOCATION
 ${CMAKE_BINARY_DIR}/../pip/lib/fp_math_bindings.lib)
add_compile_definitions(GLEW_STATIC)
add_subdirectory(src)



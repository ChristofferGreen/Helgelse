add_executable(HelgelseTest
  main.cpp
  catch.cpp
  path_space_insert.cpp
  basic_vulkan.cpp
)

target_include_directories(HelgelseTest 
  PUBLIC
    ../src
    ../ext
)

target_include_directories(HelgelseTest 
  SYSTEM
  PUBLIC
    ${DepIncludesPublic}
  PRIVATE
    ${DepIncludes}
)

target_compile_definitions(HelgelseTest
  PUBLIC
    ${DepDefines}
    CATCH_AMALGAMATED_CUSTOM_MAIN
)

target_link_libraries(HelgelseTest
  PUBLIC
    ${DepLibraries}
  PRIVATE
    Forsoning
    glfw
    Vulkan::Vulkan
)

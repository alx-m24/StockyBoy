file(GLOB_RECURSE INTERFACE_SOURCES CONFIGURE_DEPENDS
    *.cpp
    *.h
)

add_executable(Interface ${INTERFACE_SOURCES})

target_include_directories(Interface
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Headers   # fixes Application.hpp
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE LexviEngine
    PRIVATE LexviEngine/Headers
)

target_link_libraries(Interface
    PRIVATE 5PercentRuleBot
            LexviEngine
)

target_link_options(Interface PRIVATE -mconsole)

target_compile_options(Interface PRIVATE -Wno-unused-parameter)
set_target_properties(Interface PROPERTIES
    OUTPUT_NAME "StockyBoy"
)

# ---- MinGW runtime DLL copy (Windows only) ----
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    foreach(dll
        libstdc++-6.dll
        libgcc_s_seh-1.dll
        libwinpthread-1.dll
    )
        execute_process(
            COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=${dll}
            OUTPUT_VARIABLE DLL_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if(EXISTS ${DLL_PATH})
            add_custom_command(TARGET Interface POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL_PATH}"
                $<TARGET_FILE_DIR:Interface>
            )
        endif()
    endforeach()
endif()

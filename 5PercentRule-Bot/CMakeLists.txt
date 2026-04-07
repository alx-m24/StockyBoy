file(GLOB_RECURSE BOT_SOURCES CONFIGURE_DEPENDS
    *.cpp
    *.h
)
add_library(5PercentRuleBot STATIC ${BOT_SOURCES})

target_link_libraries(5PercentRuleBot
    PRIVATE LexviEngine
    PUBLIC StockScraper
)

target_include_directories(5PercentRuleBot
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Headers
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

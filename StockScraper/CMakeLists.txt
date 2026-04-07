file(GLOB_RECURSE STOCKSCRAPER_SOURCES CONFIGURE_DEPENDS
    *.cpp
    *.h
)
add_library(StockScraper STATIC ${STOCKSCRAPER_SOURCES})

target_link_libraries(StockScraper
    PRIVATE LexviEngine
    PRIVATE CURL::libcurl
    PRIVATE nlohmann_json::nlohmann_json
)

target_include_directories(StockScraper
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Headers
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}
    )

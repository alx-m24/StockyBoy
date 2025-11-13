#pragma once

#include "Types.hpp"
#include "Result.hpp"

namespace StockyBoy {
    namespace Scraper {
        size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

        Result Fetch(const std::string& label, INTERVAL interval, RANGE range, std::string& out_Data);
    }
}
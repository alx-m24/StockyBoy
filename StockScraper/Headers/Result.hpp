#pragma once

#include <string>

namespace StockyBoy {
	namespace Scraper {
        struct Result {
            bool succeeded = false;
            std::string error{};

            static Result Ok() { return { true, {} }; }
            static Result Fail(const std::string& msg) { return { false, msg }; }
        };
	}
}
#include "pch.h"

#include "Types.hpp"

namespace StockyBoy {
	std::string ToString(INTERVAL interval) {
		if (interval < 0 || interval >= INTERVAL_COUNT)
			return "unknown";
		return INTERVAL_NAMES[interval];
	}

	std::string ToString(RANGE range) {
		if (range < 0 || range >= RANGE_COUNT)
			return "unknown";
		return RANGE_NAMES[range];
	}

    INTERVAL FromStringToInterval(const std::string& str) {
        for (int i = 0; i < INTERVAL_COUNT; i++) {
            if (str == INTERVAL_NAMES[i])
                return static_cast<INTERVAL>(i);
        }
        return INTERVAL_COUNT; // invalid
    }

    RANGE FromStringToRange(const std::string& str) {
        for (int i = 0; i < RANGE_COUNT; i++) {
            if (str == RANGE_NAMES[i])
                return static_cast<RANGE>(i);
        }
        return RANGE_COUNT; // invalid
    }

    bool IsValidCombo(INTERVAL interval, RANGE range) {
        auto it = intervalRanges.find(interval);
        if (it == intervalRanges.end()) return false;
        const auto& validRanges = it->second;
        return std::find(validRanges.begin(), validRanges.end(), range) != validRanges.end();
    }
}
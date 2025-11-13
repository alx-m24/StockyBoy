#pragma once

#include <map>
#include <string>
#include <vector>

namespace StockyBoy {
    enum INTERVAL {
        MINUTES_1,
        MINUTES_2,
        MINUTES_5,
        MINUTES_15,
        MINUTES_30,
        MINUTES_60,
        DAYS_1,
        DAYS_5,
        WEEK_1,
        MONTH_1,
        MONTH_3,
        INTERVAL_COUNT
    };

    enum RANGE {
        RANGE_1D,
        RANGE_5D,
        RANGE_1MO,
        RANGE_3MO,
        RANGE_6MO,
        RANGE_1Y,
        RANGE_2Y,
        RANGE_5Y,
        RANGE_10Y,
        RANGE_YTD,
        RANGE_MAX,
        RANGE_COUNT
    };

    // For easy display and conversion
    static const char* INTERVAL_NAMES[] = {
        "1m", "2m", "5m", "15m", "30m", "60m",
        "1d", "5d", "1wk", "1mo", "3mo"
    };

    static const char* RANGE_NAMES[] = {
        "1d", "5d", "1mo", "3mo", "6mo",
        "1y", "2y", "5y", "10y", "ytd", "max"
    };

    static const std::map<INTERVAL, std::vector<RANGE>> intervalRanges = {
        {MINUTES_1, {RANGE_1D, RANGE_5D}},
        {MINUTES_2, {RANGE_1D, RANGE_5D}},
        {MINUTES_5, {RANGE_1D, RANGE_5D, RANGE_1MO}},
        {MINUTES_15, {RANGE_1D, RANGE_5D, RANGE_1MO}},
        {MINUTES_30, {RANGE_1D, RANGE_5D, RANGE_1MO}},
        {MINUTES_60, {RANGE_5D, RANGE_1MO, RANGE_3MO}},
        {DAYS_1, {RANGE_5D, RANGE_1MO, RANGE_3MO, RANGE_6MO, RANGE_1Y, RANGE_2Y, RANGE_5Y, RANGE_10Y, RANGE_YTD, RANGE_MAX}},
        {DAYS_5, {RANGE_1MO, RANGE_3MO, RANGE_6MO, RANGE_1Y, RANGE_2Y, RANGE_5Y, RANGE_10Y, RANGE_YTD, RANGE_MAX}},
        {WEEK_1, {RANGE_1MO, RANGE_3MO, RANGE_6MO, RANGE_1Y, RANGE_2Y, RANGE_5Y, RANGE_10Y, RANGE_YTD, RANGE_MAX}},
        {MONTH_1, {RANGE_3MO, RANGE_6MO, RANGE_1Y, RANGE_2Y, RANGE_5Y, RANGE_10Y, RANGE_YTD, RANGE_MAX}},
        {MONTH_3, {RANGE_1Y, RANGE_2Y, RANGE_5Y, RANGE_10Y, RANGE_MAX}},
    };

    std::string ToString(INTERVAL interval);
    std::string ToString(RANGE range);

    INTERVAL FromStringToInterval(const std::string& str);
    RANGE FromStringToRange(const std::string& str);

    bool IsValidCombo(INTERVAL interval, RANGE range);
}

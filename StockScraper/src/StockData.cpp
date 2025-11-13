#include "pch.h"

#include "StockData.hpp"
#include "Utils/StockMath.hpp"

namespace StockyBoy {
	namespace Scraper {
        Result getStockTable(const std::string& data, StockTable& table, bool normalize) {
            using json = nlohmann::json;
            json j;
            try {
                j = json::parse(data);
            }
            catch (const json::parse_error& e) {
                return Result::Fail("[StockyBoy] JSON parse error: " + std::string(e.what()));
            }

            try {
                if (!j.contains("chart") || !j["chart"].contains("result") || j["chart"]["result"].empty()) {
                    return Result::Fail("[StockyBoy] Missing chart/result in JSON.");
                }

                const auto& result = j["chart"]["result"][0];

                if (!result.contains("timestamp") || !result.contains("indicators")) {
                    return Result::Fail("[StockyBoy] Missing timestamp or indicators.");
                }

                const auto& timestamps = result["timestamp"];
                const auto& quote = result["indicators"]["quote"][0];

                const auto& openPrices = quote.value("open", json::array());
                const auto& highPrices = quote.value("high", json::array());
                const auto& lowPrices = quote.value("low", json::array());
                const auto& closePrices = quote.value("close", json::array());
                const auto& volume = quote.value("volume", json::array());

                if (timestamps.size() != openPrices.size() ||
                    timestamps.size() != highPrices.size() ||
                    timestamps.size() != lowPrices.size() ||
                    timestamps.size() != closePrices.size() ||
                    timestamps.size() != volume.size()) {
                    return Result::Fail("[StockyBoy] Mismatched array sizes in JSON data.");
                }

                const size_t rowNum = timestamps.size();

                table.data.resize(rowNum);
                table.date.resize(rowNum);
                table.open.resize(rowNum);
                table.high.resize(rowNum);
                table.low.resize(rowNum);
                table.close.resize(rowNum);
                table.volume.resize(rowNum);
                table.timeStamps.resize(rowNum);

                for (size_t i = 0; i < rowNum; ++i) {
                    time_t ts = timestamps[i];
                    struct tm tmStruct;
                    gmtime_s(&tmStruct, &ts);

                    char buf[64];
                    strftime(buf, sizeof(buf), "%Y-%m-%d", &tmStruct);

                    table.data[i] = {
                        .date = buf,
                        .open = openPrices[i].is_null() ? 0.0 : openPrices[i].get<double>(),
                        .high = highPrices[i].is_null() ? 0.0 : highPrices[i].get<double>(),
                        .low = lowPrices[i].is_null() ? 0.0 : lowPrices[i].get<double>(),
                        .close = closePrices[i].is_null() ? 0.0 : closePrices[i].get<double>(),
                        .volume = volume[i].is_null() ? 0.0 : volume[i].get<double>()
                        };

                    table.date[i] = table.data[i].date;
                    table.open[i] = table.data[i].open;
                    table.high[i] = table.data[i].high;
                    table.low[i] = table.data[i].low;
                    table.close[i] = table.data[i].close;
                    table.volume[i] = table.data[i].volume;

                    table.timeStamps[i] = static_cast<double>(i);
                }

                if (normalize) {
                    Maths::normalize(table.volume);
                }

            }
            catch (const std::exception& e) {
                return Result::Fail("[StockyBoy] Error processing JSON: " + std::string(e.what()));
            }

            return Result::Ok();
        }

	}
}
#pragma once

#include <string>
#include <vector>

#include "Result.hpp"

namespace StockyBoy {
	namespace Scraper {
		struct StockRow
		{
			std::string date = "";
			double open = 0.0;
			double high = 0.0;
			double low = 0.0;
			double close = 0.0;
			double volume = 0.0;
		};

		struct StockTable {
			std::vector<StockRow> data;

			// -- Flattenned data

			std::vector<std::string> date;
			std::vector<double> open;
			std::vector<double> high;
			std::vector<double> low;
			std::vector<double> close;
			std::vector<double> volume;

			std::vector<double> timeStamps;
		};

		Result getStockTable(const std::string& data, StockTable& table, bool normalize = false);
	}
}
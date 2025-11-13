#pragma once

#include <vector>
#include <string>

namespace StockyBoy {
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

	namespace Maths {
		std::vector<double> normalizeCopy(const std::vector<double>& data);
		void normalize(std::vector<double>& data);

		std::vector<double> SMA(const StockTable& table, uint32_t window);
	}
}
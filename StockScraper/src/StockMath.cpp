#include "pch.h"

#include "Utils/StockMath.hpp"

namespace StockyBoy {
	namespace Maths {
		std::vector<double> normalizeCopy(const std::vector<double>& data) {
			std::vector<double> result = data;
			double maxVal = *std::max_element(result.begin(), result.end());
			if (maxVal == 0.0) return result; // avoid division by zero
			for (double& val : result) val /= maxVal;
			return result;
		}

		void normalize(std::vector<double>& data) {
			double maxVal = *std::max_element(data.begin(), data.end());
			if (maxVal == 0.0) return; // avoid division by zero
			for (double& val : data) val /= maxVal;
		}

		std::vector<double> SMA(const StockTable& table, uint32_t window)
		{
			std::vector<double> SMAs(table.close.size());

			for (size_t i = static_cast<size_t>(window) - 1; i < table.close.size(); ++i) {
				double total = std::accumulate(table.close.begin() + (i - window + 1), table.close.begin() + i + 1, 0.0);
				SMAs[i] = total / window;
			}

			return SMAs;
		}
	}
}
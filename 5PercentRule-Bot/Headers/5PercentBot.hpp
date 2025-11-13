#pragma once

#include <StockScraper/Headers/Alpaca.hpp>

namespace StockyBoy {
	namespace Bots {
		namespace FivePercentRule {
			// Return True if algorithm ran
			bool Run(const std::string& logPath, StockyBoy::Scraper::Alpaca::Account& account);
		}
	}
}
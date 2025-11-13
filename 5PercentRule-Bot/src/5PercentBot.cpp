#include "pch.h"

#include "Headers/5PercentBot.hpp"

namespace StockyBoy {
	namespace Bots {
		namespace FivePercentRule {
			bool Run(const std::string& logPath, StockyBoy::Scraper::Alpaca::Account& account)
			{
				// Read log file to see if this is the first launch of the day
				// Read time to see if market is open
				// If first launch of the day & market is open, execute algorithm

				// Algorithm: Read past day profit & eliminate stocks with poor performance
				// Add new stocks so that StockNumDropped == StockNumAdded

				// For all active stocks, get past N-days data
				// Apply 5 percent rule where possible (might not be possible: insufficient funds, etc)

				// Log all transaction
				// Log current day to not re-run algorithm

				int random = rand() % 100;

				return random < 10;
			}
		}
	}
}
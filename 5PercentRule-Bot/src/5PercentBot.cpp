#include "pch.h"

#include "Headers/5PercentBot.hpp"
#include "Headers/Utils/ticker_labels.hpp"

#include <StockScraper/Headers/Fetch.hpp>
#include <StockScraper/Headers/StockData.hpp>

namespace StockyBoy {
	namespace Bots {
		namespace FivePercentRule {
			constexpr float FORGIVENESS = 0.1f;

			namespace fs = std::filesystem;
			namespace chrono = std::chrono;

			static std::string ToString(std::chrono::year_month_day ymwd) {
				using namespace std::chrono;

				year y = ymwd.year();
				month m = ymwd.month();
				day d = ymwd.day();

				// Convert to ints
				int y_val = int(y);
				unsigned m_val = unsigned(m);
				unsigned wd_val = unsigned(d);  // Sunday=0, Monday=1, ...

				return std::format("{:04}-{:02}-{:02}", y_val, m_val, wd_val);
			}

			static std::string GetTodayString() {
				auto today_days = chrono::floor<chrono::days>(chrono::system_clock::now());
				chrono::year_month_day today{ today_days };
				return ToString(today);
			}

			static bool InMarketHours() {
				using namespace std::chrono;

				auto now = system_clock::now();

				// extract date
				auto today_days = floor<days>(now);
				year_month_day ymd{ today_days };

				// check if week day
				weekday wd{ today_days };
				if (wd == Saturday || wd == Sunday) {
					return false;
				}

				// extract time of day
				auto time_since_midnight = now - today_days;
				hh_mm_ss hms{ time_since_midnight }; // in UTC

				// Convert time-of-day to seconds for easy comparison
				auto secs = duration_cast<seconds>(hms.to_duration()).count();

				// market hours
				auto open_buffer = (14h + 30min);	// latest open
				auto close_buffer = (20h);			// earliest close

				auto open_secs = duration_cast<seconds>(open_buffer).count();
				auto close_secs = duration_cast<seconds>(close_buffer).count();

				return secs >= open_secs && secs <= close_secs;
			}

			static std::vector<uint32_t> getShuffledIndices(uint32_t n) {
				std::vector<uint32_t> indices(n);
				std::iota(indices.begin(), indices.end(), 0); // fill with 0..n-1

				static thread_local std::mt19937_64 rng(std::random_device{}());
				std::shuffle(indices.begin(), indices.end(), rng);

				return indices;
			}

			static float getPercentageChange(float _old, float _new) {
				return ((_new - _old) / _old) * 100.0f;
			}

			static float getPricePercentageChange(const std::string& label, uint32_t window, float* out_CurrentPrice = nullptr) {
				using namespace StockyBoy::Scraper;

				std::string data;
				Result result = Fetch(label, DAYS_1, RANGE_1Y, data);

				if (!result.succeeded) {
					return 0.0f;
				}

				StockTable table;
				result = getStockTable(data, table);

				if (!result.succeeded) {
					return 0.0f;
				}

				const size_t recordsNum = table.close.size();

				if (recordsNum == 0 || window >= recordsNum) {
					return 0.0f;  // not enough data, or invalid window
				}

				size_t latestIndex = recordsNum - 1;
				double latestClose = table.close[latestIndex];
				double oldPrice = table.close[latestIndex - window];

				if (oldPrice == 0.0) {
					return 0.0f;
				}

				if (out_CurrentPrice) *out_CurrentPrice = latestClose;

				return getPercentageChange(oldPrice, latestClose);
			}

			using Tickers = std::unordered_map<std::string, float>;
			static Tickers getNewTrades(uint32_t window, float Budget, const std::unordered_map<std::string, float>* holding = nullptr) {
				Tickers toBuy;

				uint32_t totalTickerNum = static_cast<uint32_t>(TICKER_LABELS.size());

				auto shuffledIndices = getShuffledIndices(totalTickerNum);

				for (const uint32_t index : shuffledIndices) {
					if (Budget <= 0.0f) break;

					const std::string& label = TICKER_LABELS[index];

					if (holding && holding->contains(label)) continue;

					// If stock went down of at least 5%, buy
					float currPrice;
					if (getPricePercentageChange(label, window, &currPrice) <= -5.0f + FORGIVENESS) {
						toBuy[label] = currPrice;
						Budget -= 5.0f;
					}
				}

				return toBuy;
			}

			bool Run(const std::string& _logPath, StockyBoy::Scraper::Alpaca::Account& account, uint32_t window, float dailyBudget)
			{
				if (!InMarketHours()) {
					return false;
				}

				fs::path logPath(_logPath);

				if (!fs::exists(logPath)) {
					// Create it (creates intermediate dirs too)
					fs::create_directories(logPath);
				}

				fs::path  latestLogDataFile_path = logPath / "Latest.txt";
				std::string lastRecordDate = ""; // will stay empty if latestLogDataFile does not exist (i.e was deleted or first run)

				if (fs::exists(latestLogDataFile_path)) {
					std::ifstream latestLogDataFile(latestLogDataFile_path);
					std::getline(latestLogDataFile, lastRecordDate);
				}
				
				std::string todayDay = GetTodayString();

				if (lastRecordDate == todayDay) {
					return false;
				}

				fs::create_directories(logPath / todayDay);

				// Create/Overwrite latest.txt to write our date
				std::ofstream latestLog(latestLogDataFile_path);
				latestLog << todayDay;

				std::ofstream log(logPath / todayDay / "log.txt");

				std::unordered_map<std::string, float> tradesHolding;

				Tickers todayBuys;
				Tickers todaySells;

				if (lastRecordDate.empty()) {
					todayBuys = getNewTrades(window, dailyBudget); // get the max amount of trades for our budget cap using the 5%-rule
				}
				else {
					std::ifstream holdingTradesFile(logPath / lastRecordDate / "Holding.txt");
					std::ifstream pricesFile(logPath / lastRecordDate / "Prices.txt");
					std::string tradeLabel, price;
					while (std::getline(holdingTradesFile, tradeLabel) && std::getline(pricesFile, price)) {
						tradesHolding[tradeLabel] = std::stof(price);
					}

					for (const auto& [label, price] : tradesHolding) {
						float currPrice;
						getPricePercentageChange(label, window, &currPrice); // just to get current price

						float priceChange = getPercentageChange(price, currPrice);

						if (priceChange >= 5.0f - FORGIVENESS) {
							todaySells[label] = currPrice;
						}
					}

					// find new BUYS if budget allows it
					float totalBalance;
					if (account.GetBalance(totalBalance).succeeded) {
						if (totalBalance > dailyBudget) {
							todayBuys = getNewTrades(window, dailyBudget, &tradesHolding);
						}
					}
				}

				using namespace StockyBoy::Scraper;
				constexpr std::chrono::milliseconds tradeDelay{ 500 };

				// Execute trades
				// Unfortunately no execution::par, idk how the API will behave with that
				for (const auto& [label, price] : todayBuys) {
					bool suceeded = account.SubmitOrder(
						Order{
							.action = Action::BUY,
							.type = OrderType::MARKET,
							.value = 5.0f,
							.label = label
						}
					).succeeded;

					if (!suceeded) {
						log << "Failed to buy from " << label << '\n';
					}
					else {
						tradesHolding[label] = price;
					}

					std::this_thread::sleep_for(tradeDelay);
				}
				for (const auto& [label, price] : todaySells) {

					bool suceeded = account.SubmitOrder(
						Order{
							.action = Action::SELL,
							.type = OrderType::MARKET,
							.value = 5.0f,
							.label = label
						}
					).succeeded;

					if (!suceeded) {
						log << "Failed to sell " << label << '\n';
					}
					else {
						tradesHolding.erase(label);
					}

					std::this_thread::sleep_for(tradeDelay);
				}

				std::ofstream holdingTradesFile(logPath / todayDay / "Holding.txt");
				std::ofstream pricesTradesFile(logPath / todayDay / "Prices.txt");
				for (const auto& [label, price] : tradesHolding) {
					holdingTradesFile << label + "\n";
					pricesTradesFile << std::to_string(price) + "\n";
				}

				return true;
			}
		}
	}
}
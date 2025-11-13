#pragma once

#include "Result.hpp"

#include <map>
#include <array>
#include <string>

namespace StockyBoy {
    namespace Scraper {
        enum class Action {
            BUY,
            SELL,
            COUNT
        };

        enum class OrderType {
            MARKET,
            LIMIT,
            COUNT
        };

        struct Order {
            Action action;
            OrderType type;
        };

        namespace Alpaca { 
            enum class ACCOUNTS {
                FIVE_PERCENT,
                COUNT,
            };

            struct AlpacaAccountDetails {
                std::string uuid{};
                std::string accountNumber{};
                float cashBalance{};
                float portfolioValue{};
            };

            const inline std::array<std::string, (size_t)ACCOUNTS::COUNT> AccountName = {
                "5Percent"
            };
            class Account {
            private:
                std::string EndPoint{};
                std::string Key{};
                std::string Secret{};

                std::string Name{};

            private:
                AlpacaAccountDetails details;

            public:
                Account() = default;
                Account(const std::string& endPoint, const std::string& key, const std::string& secret);

            public:
                Result Init(const std::string& endPoint, const std::string& key, const std::string& secret);
                Result Load(ACCOUNTS account);

                Result SubmitOrder(Order order);

                Result GetBalance(float& balance);
                Result GetPorfolioValue(float& value);

                Result GetName(std::string& name);

            public:
                bool empty() {
                    return EndPoint.empty() || Secret.empty() || Key.empty();
                }
            };


        }
    }
}
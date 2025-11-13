#pragma once
#include <Game/Game.hpp>
#include <LexviEngine.hpp>

#include <mutex>
#include <string>
#include <thread>
#include <atomic>

#include "StockScraper/Headers/Types.hpp"
#include "StockScraper/Headers/Alpaca.hpp"
#include "StockScraper/Headers/StockData.hpp"

// Application
class Application : public Game {
public:
    // Core Engine Hooks
    bool loadResources(Lexvi::Engine&) override;
    void update(Lexvi::Engine& engine, float dt) override;
    void render(Lexvi::Renderer& renderer) override;
    void shutdown() override;

private:
    std::mutex algoMutex;
    std::condition_variable shutdownCV;

    std::atomic<bool> shouldStop = false;
    std::thread FivePercentThread;

private:
    // Types
    using StockTable = StockyBoy::Scraper::StockTable;
    using Account = StockyBoy::Scraper::Alpaca::Account;

    // =========================================================================
    // === Stock UI / Data =====================================================
    // =========================================================================
    struct StockUI {
        char label[10]{};
        StockyBoy::INTERVAL interval = StockyBoy::MINUTES_1;
        StockyBoy::RANGE range = StockyBoy::RANGE_1D;
        bool normalize = false;
        bool showVolume = true;
    } stockUI;

    struct StockData {
        std::string label;
        std::string rawData;
        StockTable  table;
    } stockData;

    std::mutex stockMutex;
    std::thread stockThread;

    std::atomic<bool> fetchingStock = false;

    // =========================================================================
    // === Account UI / Data ===================================================
    // =========================================================================
    struct AccountUI {
        StockyBoy::Scraper::Alpaca::ACCOUNTS selected = StockyBoy::Scraper::Alpaca::ACCOUNTS::FIVE_PERCENT;
    } accountUI;

    struct AccountData {
        Account current;
        bool available = false;
    } accountData;

    std::mutex accountMutex;
    std::thread accountThread;

    std::atomic<bool> fetchingAccount = false;

    // =========================================================================
    // === Error Handling ======================================================
    // =========================================================================
    struct ErrorHandler {
        std::vector<std::string> messages;
        bool showWindow = false;

        void Push(const std::string& err) {
            messages.push_back(err);
            showWindow = true;
        }
    } errorHandler;

    // =========================================================================
    // === UI / Navigation =====================================================
    // =========================================================================
    enum class Tab { Account, Market, Settings };
    Tab currentTab = Tab::Account;

    // =========================================================================
    // === Rendering ===========================================================
    // =========================================================================
    void RenderMainUI();

    void ErrorsWindow();

    void ShowStockPlot();
    void StockTableUI();

    // =========================================================================
    // === Async Operations ====================================================
    // =========================================================================
    void AsyncFetchData(const std::string& label, StockyBoy::INTERVAL interval,
        StockyBoy::RANGE range, bool normalize);

    void AsyncFetchAccount(StockyBoy::Scraper::Alpaca::ACCOUNTS account);

    // =========================================================================
    // === UI Theme ============================================================
    // =========================================================================
    void ApplyModernOrangeTheme();
};

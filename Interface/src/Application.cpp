#include "Application.hpp"

#include "StockScraper/Headers/Fetch.hpp"
#include "5PercentRule-Bot/Headers/5PercentBot.hpp"

#include <Utils/Logging.hpp>
#include <imgui.h>
#include <implot.h>
#include <iostream>

bool Application::loadResources(Lexvi::Engine&)
{
    this->ApplyModernOrangeTheme(); 
    shouldStop.store(false);

    FivePercentThread = std::thread(
        [this]() {
            StockyBoy::Scraper::Alpaca::Account fivePercentAccount;
            fivePercentAccount.Load(StockyBoy::Scraper::Alpaca::ACCOUNTS::FIVE_PERCENT);

            while (!shouldStop) {
                std::chrono::seconds waitDuration;
                bool algoExecuted = StockyBoy::Bots::FivePercentRule::Run("", fivePercentAccount);

                if (algoExecuted) {
                    LEXVI_LOG_INFO("[StockyBoy][5Percent] Trade cycle executed, next check in 24 hours.");
                    
                    //waitDuration = std::chrono::hours(24);
                    waitDuration = std::chrono::seconds(5);
                }
                else {
                    LEXVI_LOG_INFO("[StockyBoy][5Percent] No action taken this cycle, retrying in 30 minutes.");

                    //waitDuration = std::chrono::minutes(30);
                    waitDuration = std::chrono::seconds(1);
                }

                std::unique_lock<std::mutex> lock(algoMutex);
                shutdownCV.wait_for(lock, waitDuration, [this] { return shouldStop.load(); });
            }
        }
    );
    
    return true;
}

void Application::update(Lexvi::Engine& engine, float dt)
{
    if (currentTab == Tab::Market) {
        // disable fps limit for better graph viewing
        engine.LockFPS(-1);
    }
    else {
        engine.LockFPS(30);
    }
}

// ============================================================================
// Render Loop
// ============================================================================
void Application::render(Lexvi::Renderer& renderer) {
    RenderMainUI();
}

// ============================================================================
// Shutdown
// ============================================================================
void Application::shutdown() {
    shouldStop.store(true);
    shutdownCV.notify_all();

    if (stockThread.joinable()) stockThread.join();
    if (accountThread.joinable()) accountThread.join();
    if (FivePercentThread.joinable()) FivePercentThread.join();
}

// ============================================================================
// Async Operations
// ============================================================================
void Application::AsyncFetchData(const std::string& label,
    StockyBoy::INTERVAL interval,
    StockyBoy::RANGE range,
    bool normalize) {
    using namespace StockyBoy::Scraper;

    fetchingStock.store(true);

    std::string data;
    Result fetchResult = Fetch(label, interval, range, data);
    if (!fetchResult.succeeded) {
        errorHandler.Push(fetchResult.error);
        fetchingStock.store(false);
        this->stockData.label = "";
        return;
    }

    StockTable table;
    Result tableResult = getStockTable(data, table, normalize);
    if (!tableResult.succeeded) {
        errorHandler.Push(tableResult.error);
        fetchingStock.store(false);
        this->stockData.label = "";
        return;
    }

    std::lock_guard<std::mutex> lock(stockMutex);
    stockData.label = label;
    stockData.rawData = std::move(data);
    stockData.table = std::move(table);

    fetchingStock.store(false);
}

void Application::AsyncFetchAccount(StockyBoy::Scraper::Alpaca::ACCOUNTS account) {
    using namespace StockyBoy::Scraper;

    fetchingAccount.store(true);

    Account fetched;
    Result fetchResult = fetched.Load(account);
    if (!fetchResult.succeeded) {
        errorHandler.Push(fetchResult.error);
        fetchingAccount.store(false);
        return;
    }

    std::lock_guard<std::mutex> lock(accountMutex);
    accountData.current = std::move(fetched);
    accountData.available = true;

    fetchingAccount.store(false);
}

// ============================================================================
// UI - Main App UI
// ============================================================================
void Application::RenderMainUI() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("StockyBoy", nullptr, windowFlags);

    if (ImGui::BeginTabBar("MainTabBar", ImGuiTabBarFlags_None)) {
        // --------------------------------------------------------------------
        // ACCOUNT TAB
        // --------------------------------------------------------------------
        if (ImGui::BeginTabItem("Account")) {
            currentTab = Tab::Account;

            using namespace StockyBoy::Scraper::Alpaca;

            ImGui::SeparatorText("Account Selection");
            const std::string currentStr = AccountName[(size_t)accountUI.selected];
            if (ImGui::BeginCombo("Account", currentStr.c_str())) {
                for (size_t i = 0; i < (size_t)ACCOUNTS::COUNT; i++) {
                    bool selected = (accountUI.selected == static_cast<ACCOUNTS>(i));
                    if (ImGui::Selectable(AccountName[i].c_str(), selected))
                        accountUI.selected = static_cast<ACCOUNTS>(i);
                    if (selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            if (ImGui::Button("Fetch Account")) {
                if (accountThread.joinable()) accountThread.join();
                accountThread = std::thread(&Application::AsyncFetchAccount, this, accountUI.selected);
            }

            if (fetchingAccount.load()) {
                ImGui::ProgressBar((float)ImGui::GetTime() * -0.2f, ImVec2(-1, 0), "Fetching...");
            }

            // Show account data
            if (accountData.available) {
                std::string name;
                float balance = 0.f, portfolioValue = 0.f;
                accountData.current.GetName(name);
                accountData.current.GetBalance(balance);
                accountData.current.GetPorfolioValue(portfolioValue);

                ImGui::SeparatorText("Account Summary");
                ImGui::Text("Name: %s", name.c_str());
                ImGui::Text("Balance: %.2f", balance);
                ImGui::Text("Portfolio Value: %.2f", portfolioValue);
            }

            ImGui::EndTabItem();
        }

        // --------------------------------------------------------------------
        // MARKET TAB
        // --------------------------------------------------------------------
        if (ImGui::BeginTabItem("Market")) {
            currentTab = Tab::Market;

            using namespace StockyBoy;

            // --- Stock fetch options ---
            ImGui::SeparatorText("Stock Fetch Options");

            ImGui::InputText("Label", stockUI.label, IM_ARRAYSIZE(stockUI.label));

            const std::string intervalStr = ToString(stockUI.interval);
            if (ImGui::BeginCombo("Interval", intervalStr.c_str())) {
                for (int i = 0; i < INTERVAL_COUNT; i++) {
                    INTERVAL interval = static_cast<INTERVAL>(i);
                    const std::string name = ToString(interval);
                    bool selected = (stockUI.interval == interval);
                    if (ImGui::Selectable(name.c_str(), selected)) {
                        stockUI.interval = interval;
                        const auto& availableRanges = intervalRanges.at(stockUI.interval);
                        if (!availableRanges.empty())
                            stockUI.range = availableRanges.front();
                    }
                    if (selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            const std::string rangeStr = ToString(stockUI.range);
            const auto& ranges = intervalRanges.at(stockUI.interval);
            if (ImGui::BeginCombo("Since", rangeStr.c_str())) {
                for (auto range : ranges) {
                    const std::string name = ToString(range);
                    bool selected = (stockUI.range == range);
                    if (ImGui::Selectable(name.c_str(), selected))
                        stockUI.range = range;
                    if (selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::Checkbox("Normalize Data", &stockUI.normalize);

            if (ImGui::Button("Fetch Stock Data")) {
                if (stockThread.joinable()) stockThread.join();
                stockThread = std::thread(&Application::AsyncFetchData, this,
                    std::string(stockUI.label),
                    stockUI.interval, stockUI.range,
                    stockUI.normalize);
            }

            if (fetchingStock.load()) {
                ImGui::ProgressBar((float)ImGui::GetTime() * -0.2f, ImVec2(-1, 0), "Fetching...");
            }

            ImGui::SeparatorText("Stock Overview");
            ImGui::Checkbox("Show Volume", &stockUI.showVolume);

            // --- Plot + Table in two child regions ---
            ImGui::BeginChild("PlotRegion", ImVec2(0, viewport->Size.y * 0.45f), true);
            ShowStockPlot();
            ImGui::EndChild();

            ImGui::Separator();
            ImGui::BeginChild("TableRegion", ImVec2(0, 0), true);
            StockTableUI();
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        // --------------------------------------------------------------------
        // SETTINGS TAB
        // --------------------------------------------------------------------
        if (ImGui::BeginTabItem("Settings")) {
            currentTab = Tab::Settings;
            ImGui::Text("Settings coming soon...");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    // Error display docked at bottom of screen
    if (errorHandler.showWindow && !errorHandler.messages.empty()) {
        ImGui::SeparatorText("Errors");
        for (const auto& err : errorHandler.messages)
            ImGui::TextUnformatted(err.c_str());
        if (ImGui::Button("Clear Errors"))
            errorHandler.messages.clear();
    }

    ImGui::End(); // StockyBoy main window
}

// ============================================================================
// UI - Stock Table
// ============================================================================
void Application::StockTableUI() {
    if (stockData.label.empty()) return;

    if (ImGui::BeginTable("Stocks", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Date");
        ImGui::TableSetupColumn("Open");
        ImGui::TableSetupColumn("High");
        ImGui::TableSetupColumn("Low");
        ImGui::TableSetupColumn("Close");
        ImGui::TableSetupColumn("Volume");
        ImGui::TableHeadersRow();

        for (const auto& row : stockData.table.data) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("%s", row.date.c_str());
            ImGui::TableNextColumn(); ImGui::Text("%.2f", row.open);
            ImGui::TableNextColumn(); ImGui::Text("%.2f", row.high);
            ImGui::TableNextColumn(); ImGui::Text("%.2f", row.low);
            ImGui::TableNextColumn(); ImGui::Text("%.2f", row.close);
            ImGui::TableNextColumn(); ImGui::Text("%.2f", row.volume);
        }

        ImGui::EndTable();
    }
}

// ============================================================================
// UI - Stock Plot
// ============================================================================
void Application::ShowStockPlot() {
    if (ImPlot::BeginPlot("Stock Overview", ImVec2(-1, -1))) {
        if (!stockData.table.close.empty()) {
            ImPlot::SetupAxis(ImAxis_X1, "Day");
            ImPlot::SetupAxis(ImAxis_Y1, "Price ($)");
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, (double)stockData.table.data.size() - 1);

            if (stockUI.showVolume)
                ImPlot::SetupAxis(ImAxis_Y2, "Volume", ImPlotAxisFlags_AuxDefault);

            ImPlot::SetNextLineStyle(ImVec4(0, 0.8f, 0, 1));
            ImPlot::PlotLine("Close Price",
                stockData.table.timeStamps.data(),
                stockData.table.close.data(),
                (int)stockData.table.close.size());

            if (stockUI.showVolume) {
                ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
                ImPlot::SetNextFillStyle(ImVec4(0.3f, 0.5f, 1.0f, 0.4f));
                ImPlot::PlotBars("Volume",
                    stockData.table.timeStamps.data(),
                    stockData.table.volume.data(),
                    (int)stockData.table.volume.size(), 0.5);
                ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
            }
        }
        ImPlot::EndPlot();
    }
}


// ============================================================================
// UI - Error Window
// ============================================================================
void Application::ErrorsWindow() {
    if (!errorHandler.showWindow) return;

    if (!ImGui::Begin("Errors", &errorHandler.showWindow)) {
        ImGui::End();
        return;
    }

    for (const auto& err : errorHandler.messages)
        ImGui::TextUnformatted(err.c_str());

    if (ImGui::Button("Clear")) {
        errorHandler.messages.clear();
        errorHandler.showWindow = false;
    }

    ImGui::End();
}

// ============================================================================
// - UI Theme
// ============================================================================
void Application::ApplyModernOrangeTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Base dark theme
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.25f, 0.60f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);

    // Orange accents
    ImVec4 accent = ImVec4(1.00f, 0.55f, 0.10f, 1.00f);
    ImVec4 accentHover = ImVec4(1.00f, 0.65f, 0.25f, 1.00f);

    colors[ImGuiCol_Header] = accent;
    colors[ImGuiCol_HeaderHovered] = accentHover;
    colors[ImGuiCol_HeaderActive] = accent;
    colors[ImGuiCol_Button] = accent;
    colors[ImGuiCol_ButtonHovered] = accentHover;
    colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.40f, 0.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = accent;
    colors[ImGuiCol_SliderGrabActive] = accentHover;
    colors[ImGuiCol_CheckMark] = accent;
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered] = accentHover;
    colors[ImGuiCol_TabActive] = accent;
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

    // Optional tweaks
    style.FrameRounding = 5.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 5.0f;
    style.ScrollbarRounding = 6.0f;
    style.WindowRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.FrameBorderSize = 1.0f;
}
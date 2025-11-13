#include "pch.h"

#include "Fetch.hpp"

namespace StockyBoy {
    namespace Scraper {
        size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        }

        Result Fetch(const std::string& label, INTERVAL interval, RANGE range, std::string& out_Data)
        {
            // --- Validate inputs ---
            if (label.empty()) {
                return Result::Fail("[StockyBoy][Fetch] Stock label is empty.");
            }

            if (!StockyBoy::IsValidCombo(interval, range)) {
                return Result::Fail("[StockyBoy][Fetch] Invalid interval-range combo: " +
                    StockyBoy::ToString(interval) + " / " + StockyBoy::ToString(range));
            }

            // --- Build URL ---
            const std::string url =
                "https://query1.finance.yahoo.com/v8/finance/chart/" + label +
                "?interval=" + StockyBoy::ToString(interval) +
                "&range=" + StockyBoy::ToString(range);

            // --- Initialize CURL ---
            CURL* curl = curl_easy_init();
            if (!curl) {
                return Result::Fail("[StockyBoy][Fetch] Failed to initialize CURL");
            }

            // --- Cleanup scope ---
            auto cleanup = [&]() {
                curl_easy_cleanup(curl);
                };

            // --- Configure CURL ---
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out_Data);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0"); // Some APIs block curl’s default UA
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);              // 10s timeout
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);        // Handle redirects

            // --- Perform request ---
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                cleanup();
                return Result::Fail("[StockyBoy][Fetch] CURL request failed: " + std::string(curl_easy_strerror(res)));
            }

            // --- Check HTTP response ---
            long httpCode = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            if (httpCode != 200) {
                cleanup();
                return Result::Fail("[StockyBoy][Fetch] HTTP error code: " + std::to_string(httpCode));
            }

            // --- Everything succeeded ---
            cleanup();
            return Result::Ok();
        }

    }
}
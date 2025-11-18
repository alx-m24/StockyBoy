#include "pch.h"

#include "Alpaca.hpp"

#include "Result.hpp"
#include "Fetch.hpp"

static std::string Trim(const std::string& s) {
	size_t start = s.find_first_not_of(" \t");
	size_t end = s.find_last_not_of(" \t");
	return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

namespace StockyBoy {
	namespace Scraper {
		namespace Alpaca {

			Account::Account(const std::string& endPoint, const std::string& key, const std::string& secret)
			{
				this->Init(endPoint, key, secret);
			}

			Result Account::Init(const std::string& endPoint, const std::string& key, const std::string& secret)
			{
				this->Key = key;
				this->EndPoint = endPoint;
				this->Secret = secret;

				CURL* curl = curl_easy_init();
				if (!curl) {
					return Result::Fail("[StockyBoy][Alapaca] Failed to init Curl");
				}

				struct curl_slist* headers = nullptr;
				headers = curl_slist_append(headers, ("APCA-API-KEY-ID: " + Key).c_str());
				headers = curl_slist_append(headers, ("APCA-API-SECRET-KEY: " + Secret).c_str());
				headers = curl_slist_append(headers, "accept: application/json");

				auto cleanup = [&]() {
					curl_slist_free_all(headers);
					curl_easy_cleanup(curl);
					};

				std::string response;
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
				curl_easy_setopt(curl, CURLOPT_URL, (EndPoint + "/account").c_str());
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
				curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // 10-second timeout

				CURLcode res = curl_easy_perform(curl);
				if (res != CURLE_OK) {
					cleanup();
					return Result::Fail("[StockyBoy][Alpaca] request failed: Curl error Code " + std::to_string(res));
				}

				long httpCode = 0;
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
				if (httpCode != 200) {
					cleanup();
					return Result::Fail("[StockyBoy][Alpaca] HTTP error code: " + std::to_string(httpCode));
				}

				try {
					nlohmann::json j = nlohmann::json::parse(response);

					this->details = AlpacaAccountDetails{
						.uuid = j.at("id").get<std::string>(),
						.accountNumber = j.at("account_number").get<std::string>(),
						.cashBalance = std::stof(j.at("cash").get<std::string>()),
						.portfolioValue = std::stof(j.at("equity").get<std::string>())
					};
				}
				catch (const std::exception& ex) {
					cleanup();
					return Result::Fail("Failed to parse JSON: " + std::string(ex.what()));
				}

				cleanup();

				return Result::Ok(); // everything went fine
			}


			Result Account::Load(ACCOUNTS account)
			{
				std::string LocalPath = std::filesystem::current_path().string() + "\\src\\";

				std::string accountName = AccountName[static_cast<size_t>(account)];
				std::string Creditentials = "CREDITENTIALS-" + accountName + ".txt";

				std::string path = LocalPath + Creditentials;

				this->Name = accountName;

				if (!std::filesystem::exists(path)) {
					return Result::Fail("[StockyBoy][Alapaca] Can't find account creditentials for: " + accountName);
				}

				std::ifstream file(path);

				std::string endPoint;
				std::string key;
				std::string secret;

				if (!std::getline(file, endPoint)) {
					return Result::Fail("[StockyBoy][Alapaca] Can't read endpoint for: " + accountName);
				}
				if (!std::getline(file, key)) {
					return Result::Fail("[StockyBoy][Alapaca] Can't read key for: " + accountName);
				}
				if (!std::getline(file, secret)) {
					return Result::Fail("[StockyBoy][Alapaca] Can't read secret for: " + accountName);
				}

				// Remove prefixes
				const std::string endpointPrefix = "Endpoint: ";
				const std::string keyPrefix = "Key: ";
				const std::string secretPrefix = "Secret: ";

				if (endPoint.find(endpointPrefix) == 0) {
					endPoint = Trim(endPoint.substr(endpointPrefix.length()));
				}
				if (key.find(keyPrefix) == 0) {
					key = Trim(key.substr(keyPrefix.length()));
				}
				if (secret.find(secretPrefix) == 0) {
					secret = Trim(secret.substr(secretPrefix.length()));
				}

				return this->Init(endPoint, key, secret);
			}

			Result Account::SubmitOrder(Order order)
			{
				if (this->empty()) {
					return Result::Fail("[StockyBoy][Alapaca] Account Wrongly/Not fully initialized");
				}

				CURL* curl = curl_easy_init();
				if (!curl) {
					return Result::Fail("[StockyBoy][Alapaca] Failed to init Curl");
				}

				struct curl_slist* headers = nullptr;
				headers = curl_slist_append(headers, ("APCA-API-KEY-ID: " + Key).c_str());
				headers = curl_slist_append(headers, ("APCA-API-SECRET-KEY: " + Secret).c_str());
				headers = curl_slist_append(headers, "accept: application/json");
				headers = curl_slist_append(headers, "content-type: application/json");

				auto cleanup = [&]() {
					curl_slist_free_all(headers);
					curl_easy_cleanup(curl);
					};

				// Build JSON body
				std::string body = "{";
				body += "\"type\":\"" + std::string(order.type == OrderType::MARKET ? "market" : "limit") + "\",";
				body += "\"time_in_force\":\"day\",";
				body += "\"symbol\":\"" + order.label + "\",";
				body += "\"notional\":\"" + std::to_string(order.value) + "\",";
				body += "\"side\":\"" + std::string(order.action == Action::BUY ? "buy" : "sell") + "\"";
				body += "}";

				std::string response;

				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
				curl_easy_setopt(curl, CURLOPT_URL, (EndPoint + "/orders").c_str());
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
				curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

				// POST setup
				curl_easy_setopt(curl, CURLOPT_POST, 1L);
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

				CURLcode res = curl_easy_perform(curl);
				if (res != CURLE_OK) {
					cleanup();
					return Result::Fail("[StockyBoy][Alpaca] Curl error: " + std::string(curl_easy_strerror(res)));
				}

				long httpCode = 0;
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

				if (httpCode < 200 || httpCode >= 300) {
					cleanup();
					return Result::Fail("[StockyBoy][Alpaca] HTTP error " + std::to_string(httpCode) +
						" | Response: " + response);
				}

				cleanup();
				return Result::Ok();
			}

			Result Account::GetBalance(float& balance)
			{
				if (this->empty()) {
					return Result::Fail("[StockyBoy][Alapaca] Account Wrongly/Not fully initialized");
				}

				balance = this->details.cashBalance;

				return Result::Ok();
			}

			Result Account::GetPorfolioValue(float& value)
			{
				if (this->empty()) {
					return Result::Fail("[StockyBoy][Alapaca] Account Wrongly/Not fully initialized");
				}

				value = this->details.portfolioValue;

				return Result::Ok();
			}

			Result Account::GetName(std::string& name)
			{
				if (this->empty()) {
					return Result::Fail("[StockyBoy][Alapaca] Account Wrongly/Not fully initialized");
				}

				name = this->Name;

				return Result::Ok();
			}

		}
	}
}
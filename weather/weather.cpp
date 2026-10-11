#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

size_t write_callback(
    void* contents,
    size_t size,
    size_t nmemb,
    std::string* output
)
{
    size_t total = size * nmemb;
    output->append(static_cast<char*>(contents), total);
    return total;
}

int main(int argc, char* argv[])
{
    const std::vector<std::string> all_campuses = {
        "handan", "jiangwan", "zhangjiang", "fenglin"
    };

    std::vector<std::string> campuses;
    if (argc == 1)
        campuses = all_campuses;
    else
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg.rfind("--", 0) != 0 || !std::count(
                    all_campuses.begin(), all_campuses.end(), arg.substr(2)))
            {
                std::cerr << "Bad Input." << std::endl;
                return 1;
            }
            campuses.push_back(arg.substr(2));
        }
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return 1;
    }
    std::string response;
    const char* url =
        "https://aos.fudan.edu.cn/_web/_customizes/fudan/weather/api/today.rst";
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    curl_easy_setopt(curl, CURLOPT_REFERER, "https://aos.fudan.edu.cn/");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode result = curl_easy_perform(curl);
    if (result != CURLE_OK)
    {
        std::cerr << "CURL request failed: " << curl_easy_strerror(result) << std::endl;
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 1;
    }

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if (http_code != 200)
    {
        std::cerr << "Request failed: HTTP " << http_code << std::endl;
        return 1;
    }

    try
    {
        auto j = json::parse(response);
        for (const auto& campus : campuses)
        {
            const auto& entries = j.at("data").at(campus);
            std::cout << entries.at(0).at("campusName").get<std::string>()
                      << std::endl;
            for (const auto& weather : entries)
            {
                std::cout << weather.at("weatherDate").get<std::string>() << " "
                          << weather.at("weatherName").get<std::string>() << " "
                          << weather.at("tempLow").get<int>() << " ~ "
                          << weather.at("tempHigh").get<int>() << " °C, "
                          << weather.at("humidity").get<std::string>() << ", "
                          << weather.at("windDirection").get<std::string>() << " "
                          << weather.at("windLevel").get<std::string>() << std::endl;
            }
        }
    }
    catch (const nlohmann::json::exception& e)
    {
        std::cerr << "Failed to parse weather data: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
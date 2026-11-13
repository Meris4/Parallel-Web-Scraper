#include "DownloaderSerial.hpp"
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <thread>
#include <chrono>


/**
 * Callback function used by cURL to write received data into a std::string.
 *
 * ptr - Pointer to the received data buffer.
 * size - Size of each data element.
 * nmemb - Number of data elements.
 * userdata - Pointer to user data (expected to be a std::string*).
 * @return Number of bytes successfully handled (should be size * nmemb).
 */
size_t writeCallbackSerial(char* ptr, size_t size, size_t nmemb, void* userdata) {
    if (!userdata) return 0;
    std::string* str = static_cast<std::string*>(userdata);
    str->append(ptr, size * nmemb);
    return size * nmemb;
}


/**
 * Downloads the content from the given URL using cURL, with timeout and retry logic.
 *
 * url - The URL to fetch.
 * timeout - Maximum time in seconds to wait for the server response.
 * maxRetries - Maximum number of retry attempts if the request fails.
 * @return  A std::string containing the full response body (e.g., HTML content) received from the server.
 *          If all attempts fail or no data is received, returns an empty string.
 */
std::string fetchUrlSerial(const std::string& url, long timeout, int maxRetries) {
    CURL* curl;
    CURLcode res;
    std::string response;
    int attempts = 0;
    const double retryDelaySec = 0.5;

    do {
        std::cout << "fetchUrlSerial: " << url << std::endl;
        curl = curl_easy_init();
        if (curl) {
            std::cout << "curl_easy_init OK" << std::endl;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallbackSerial);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
            std::cout << "curl_easy_perform START" << std::endl;
            res = curl_easy_perform(curl);
            std::cout << "curl_easy_perform END" << std::endl;
            curl_easy_cleanup(curl);

            if (res == CURLE_OK) {
                break;
            }
            else {
                response.clear();
                std::this_thread::sleep_for(std::chrono::duration<double>(retryDelaySec));
            }
        }
        attempts++;
    } while (attempts < maxRetries);

    return response;
}


/**
 * Downloads multiple web pages sequentially and stores their HTML content.

 * urls - Vector of URLs to download.
 * htmlPages - Vector where the downloaded HTML pages will be stored.
 * visitedUrls - Map to track which URLs have been visited.
 * @return void
 */

void downloadPagesSerial(
    const std::vector<std::string>& urls,
    std::vector<std::string>& htmlPages,
    std::unordered_map<std::string, bool>& visitedUrls
) {
    for (size_t i = 0; i < urls.size(); ++i) {
        visitedUrls[urls[i]] = true;
        std::string html = fetchUrlSerial(urls[i], 10, 3);
        if (i == 0) {
            std::ofstream out("debug_downloaded.html");
            out << html;
            out.close();
        }
        htmlPages.push_back(std::move(html));
    }
}
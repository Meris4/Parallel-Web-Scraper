#include "Downloader.hpp"
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <tbb/tick_count.h>
#include <tbb/parallel_for.h>


/**
 * Callback function used by cURL to write received data into a std::string.
 *
 * ptr - Pointer to the received data buffer.
 * size - Size of each data element.
 * nmemb - Number of data elements.
 * userdata - Pointer to user data (expected to be a std::string*).
 * @return Number of bytes successfully handled (should be size * nmemb).
 */
static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
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
 * @return A std::string containing the full response body (e.g., HTML content) received from the server.
 *         If all attempts fail or no data is received, returns an empty string. 
 */
std::string fetchUrl(const std::string& url, long timeout, int maxRetries) {
    CURL* curl;
    CURLcode res;
    std::string response;
    int attempts = 0;
    const double retryDelaySec = 0.5;

    do {
        std::cout << "fetchUrl: " << url << std::endl;
        curl = curl_easy_init();
        if (curl) {
            std::cout << "curl_easy_init OK" << std::endl;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());                   // Set the URL to fetch
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);       // Specify the callback function to handle received data
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);               // Provide the buffer to store the downloaded content
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);                   // Set the maximum time allowed for the request (timeout)
            std::cout << "curl_easy_perform START" << std::endl;
            res = curl_easy_perform(curl);
            std::cout << "curl_easy_perform END" << std::endl;
            if (res != CURLE_OK) {
                std::cerr << "cURL erorr for " << url << ": " << curl_easy_strerror(res) << std::endl;
            }
            curl_easy_cleanup(curl);

            if (res == CURLE_OK) {
                break;
            }
            else {
                response.clear();
                tbb::tick_count start = tbb::tick_count::now();
                while ((tbb::tick_count::now() - start).seconds() < retryDelaySec) {
                    // active waiting
                }
            }
        }
        attempts++;
    } while (attempts < maxRetries);

    return response;
}

/**
 * Downloads multiple web pages in parallel and stores their HTML content.
 *
 * urls - A concurrent vector of URLs to download.
 * htmlPages - A concurrent vector where the downloaded HTML pages will be stored.
 * visitedUrls - A concurrent hash map to track which URLs have been visited.
 * @return void
 */
void downloadPages(
    const tbb::concurrent_vector<std::string>& urls,
    tbb::concurrent_vector<std::string>& htmlPages,
    tbb::concurrent_hash_map<std::string, bool>& visitedUrls
) {
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, urls.size(), 10),
        [&](const tbb::blocked_range<size_t>& r) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                visitedUrls.insert({ urls[i], true });
                std::string html = fetchUrl(urls[i], 10, 3);
                htmlPages.push_back(std::move(html));
            }
        }
    );
}
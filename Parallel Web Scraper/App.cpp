#include "App.hpp"
#include "BookInfo.hpp"
#include "HtmlParser.hpp"
#include "Downloader.hpp"
#include "Statistics.hpp"
#include "HtmlParserSerial.hpp"
#include "DownloaderSerial.hpp"
#include "StatisticsSerial.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <chrono>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/tick_count.h>
#include <curl/curl.h>


void runParallel() {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    tbb::concurrent_vector<std::string> urls;
    parallelIndexSite("http://books.toscrape.com/catalogue/page-1.html", urls);

    tbb::concurrent_vector<std::string> htmlPages;
    tbb::concurrent_hash_map<std::string, bool> visitedUrls;
    tbb::concurrent_vector<BookInfo> books;

    tbb::tick_count t0 = tbb::tick_count::now();

    downloadPages(urls, htmlPages, visitedUrls);
    parseAllPages(htmlPages, books);

    int count5Stars = 0;
    std::string longestTitle, shortestTitle;
    double maxPrice = -1, minPrice = 1e9;
    std::string maxTitle, minTitle;
    int count1Star = 0;
    double avgPrice = 0.0; 

    calculateStatistics(
        books,
        count5Stars,
        longestTitle,
        shortestTitle,
        maxPrice,
        maxTitle,
        minPrice,
        minTitle,
        count1Star,
        avgPrice 
    );


    tbb::tick_count t1 = tbb::tick_count::now();
    double seconds = (t1 - t0).seconds();
    double throughput = htmlPages.size() / seconds;

    writeResults(books, count5Stars, longestTitle, shortestTitle, htmlPages.size(), visitedUrls.size(), seconds, throughput,
        avgPrice, maxTitle, maxPrice, minTitle, minPrice, count1Star);

    curl_global_cleanup();

    
    std::cout << "Total number of books: " << books.size() << std::endl;
    std::cout << "Number of books with 5 stars: " << count5Stars << std::endl;
    std::cout << "Average book price: " << avgPrice << std::endl;
    std::cout << "Most expensive book: " << maxTitle << " (" << maxPrice << ")" << std::endl;
    std::cout << "Cheapest book: " << minTitle << " (" << minPrice << ")" << std::endl;
    std::cout << "Number of books with 1 star: " << count1Star << std::endl;
    std::cout << "Longest title: " << longestTitle << std::endl;
    std::cout << "Shortest title: " << shortestTitle << std::endl;
    std::cout << "Number of downloaded pages: " << htmlPages.size() << std::endl;
    std::cout << "Number of unique URLs: " << visitedUrls.size() << std::endl;
    std::cout << "Total time: " << seconds << " seconds" << std::endl;
    std::cout << "Throughput: " << throughput << " pages/second" << std::endl;
}

void runSerial() {
    std::vector<std::string> urls;
    indexSiteSerial("http://books.toscrape.com/catalogue/page-1.html", urls);

    std::vector<std::string> htmlPages;
    std::unordered_map<std::string, bool> visitedUrls;
    std::vector<BookInfo> books;

    auto t0 = std::chrono::high_resolution_clock::now();

    downloadPagesSerial(urls, htmlPages, visitedUrls);
    parseAllPagesSerial(htmlPages, books);

    int count5Stars = 0;
    std::string longestTitle, shortestTitle;
    double maxPrice = -1, minPrice = 1e9;
    std::string maxTitle, minTitle;
    int count1Star = 0;
    double avgPrice = 0.0;

    calculateStatisticsSerial(
        books,
        count5Stars,
        longestTitle,
        shortestTitle,
        maxPrice,
        maxTitle,
        minPrice,
        minTitle,
        count1Star,
        avgPrice
    );

    auto t1 = std::chrono::high_resolution_clock::now();
    double seconds = std::chrono::duration<double>(t1 - t0).count();
    double throughput = htmlPages.size() / seconds;

    writeResultsSerial(books, count5Stars, longestTitle, shortestTitle, htmlPages.size(), visitedUrls.size(), seconds, throughput,
        avgPrice, maxTitle, maxPrice, minTitle, minPrice, count1Star);

    
    std::cout << "Total number of books: " << books.size() << std::endl;
    std::cout << "Number of books with 5 stars: " << count5Stars << std::endl;
    std::cout << "Average book price: " << avgPrice << std::endl;
    std::cout << "Most expensive book: " << maxTitle << " (" << maxPrice << ")" << std::endl;
    std::cout << "Cheapest book: " << minTitle << " (" << minPrice << ")" << std::endl;
    std::cout << "Number of books with 1 star: " << count1Star << std::endl;
    std::cout << "Longest title: " << longestTitle << std::endl;
    std::cout << "Shortest title: " << shortestTitle << std::endl;
    std::cout << "Number of downloaded pages: " << htmlPages.size() << std::endl;
    std::cout << "Number of unique URLs: " << visitedUrls.size() << std::endl;
    std::cout << "Total time: " << seconds << " seconds" << std::endl;
    std::cout << "Throughput: " << throughput << " pages/second" << std::endl;
}
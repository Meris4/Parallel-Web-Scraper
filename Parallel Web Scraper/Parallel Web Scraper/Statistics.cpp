#include "Statistics.hpp"
#include <fstream>
#include <tbb/parallel_reduce.h>
#include <limits>

/**
* Writes the calculated statistics and results to a text file("webScraper.txt").
*
* books - A concurrent vector of all BookInfo objects.
* count5stars - Number of books with 5 - star rating.
* longestTitle - The longest book title.
* shortestTitle - The shortest book title.
* numPages - Number of downloaded pages.
* numUniqueUrls - Number of unique URLs.
* seconds - Total time taken for the operation(in seconds).
* throughput - Throughput in pages per second.
* avgPrice - Average book price.
* maxTitle - Title of the most expensive book.
* maxPrice - Price of the most expensive book.
* minTitle - Title of the cheapest book.
* minPrice - Price of the cheapest book.
* count1Star - Number of books with 1 - star rating.
* @return void
*/
void writeResults(
    const tbb::concurrent_vector<BookInfo>& books,
    int count5stars,
    const std::string& longestTitle,
    const std::string& shortestTitle,
    size_t numPages,
    size_t numUniqueUrls,
    double seconds,
    double throughput,
    double avgPrice,
    const std::string& maxTitle,
    double maxPrice,
    const std::string& minTitle,
    double minPrice,
    int count1Star
) {
    std::ofstream out("webScraper.txt");
    out << "Total number of books: " << books.size() << "\n";
    out << "Number of books with 5 stars: " << count5stars << "\n";
    out << "Average book price: " << avgPrice << "\n";
    out << "Most expensive book: " << maxTitle << " (" << maxPrice << ")\n";
    out << "Cheapest book: " << minTitle << " (" << minPrice << ")\n";
    out << "Number of books with 1 star: " << count1Star << "\n";
    out << "Longest title: " << longestTitle << "\n";
    out << "Shortest title: " << shortestTitle << "\n";
    out << "Number of downloaded pages: " << numPages << "\n";
    out << "Number of unique URLs: " << numUniqueUrls << "\n";
    out << "Total time: " << seconds << " seconds\n";
    out << "Throughput: " << throughput << " pages/second\n";
    out.close();
}


struct StatsAccumulator {
    int count5Stars = 0;
    int count1Star = 0;
    double maxPrice = -1;
    std::string maxTitle;
    double minPrice = 1e9;
    std::string minTitle;
    double sum = 0.0;
    size_t max_len = 0;
    std::string longestTitle;
    size_t min_len = std::numeric_limits<size_t>::max();
    std::string shortestTitle;

    StatsAccumulator() {}

    StatsAccumulator(StatsAccumulator& s, tbb::split)
        : count5Stars(0), count1Star(0), maxPrice(-1), minPrice(1e9), sum(0.0),
        max_len(0), min_len(std::numeric_limits<size_t>::max()) {
    }

    void operator()(const tbb::blocked_range<size_t>& r, const tbb::concurrent_vector<BookInfo>& books) {
        for (size_t i = r.begin(); i != r.end(); ++i) {
            const auto& book = books[i];
            if (book.rating == 5) count5Stars++;
            if (book.rating == 1) count1Star++;
            if (book.title.length() > max_len) {
                max_len = book.title.length();
                longestTitle = book.title;
            }
            if (book.title.length() < min_len) {
                min_len = book.title.length();
                shortestTitle = book.title;
            }
            if (book.price > maxPrice) {
                maxPrice = book.price;
                maxTitle = book.title;
            }
            if (book.price < minPrice) {
                minPrice = book.price;
                minTitle = book.title;
            }
            sum += book.price;
        }
    }

    void join(const StatsAccumulator& rhs) {
        count5Stars += rhs.count5Stars;
        count1Star += rhs.count1Star;
        if (rhs.maxPrice > maxPrice) {
            maxPrice = rhs.maxPrice;
            maxTitle = rhs.maxTitle;
        }
        if (rhs.minPrice < minPrice) {
            minPrice = rhs.minPrice;
            minTitle = rhs.minTitle;
        }
        if (rhs.max_len > max_len) {
            max_len = rhs.max_len;
            longestTitle = rhs.longestTitle;
        }
        if (rhs.min_len < min_len) {
            min_len = rhs.min_len;
            shortestTitle = rhs.shortestTitle;
        }
        sum += rhs.sum;
    }
};

/**
 * Helper structure for accumulating statistics about books in parallel.
 * Used with TBB's parallel_reduce.
 */
void calculateStatistics(
    const tbb::concurrent_vector<BookInfo>& books,
    int& count5Stars,
    std::string& longestTitle,
    std::string& shortestTitle,
    double& maxPrice,
    std::string& maxTitle,
    double& minPrice,
    std::string& minTitle,
    int& count1Star,
    double& avgPrice
) {
    StatsAccumulator result = tbb::parallel_reduce(
        tbb::blocked_range<size_t>(0, books.size()),
        StatsAccumulator(),
        [&](const tbb::blocked_range<size_t>& r, StatsAccumulator acc) {
            acc.operator()(r, books);
            return acc;
        },
        [](const StatsAccumulator& a, const StatsAccumulator& b) {
            StatsAccumulator res = a;
            res.join(b);
            return res;
        }
    );

    count5Stars = result.count5Stars;
    longestTitle = result.longestTitle;
    shortestTitle = result.shortestTitle;
    maxPrice = result.maxPrice;
    maxTitle = result.maxTitle;
    minPrice = result.minPrice;
    minTitle = result.minTitle;
    count1Star = result.count1Star;
    avgPrice = books.size() ? (result.sum / books.size()) : 0.0;
}


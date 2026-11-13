#include "StatisticsSerial.hpp"
#include <fstream>
#include <limits>
#include <vector>
#include <string>


/**
 * Calculates various statistics about books from the given vector, in a sequential manner.
 *
 * books - Vector containing information about books (BookInfo).
 *         Each element holds the title, price, currency, and rating of a book.
 * count5Stars – the number of books with a 5-star rating.
 * longestTitle – the longest book title found in the collection.
 * shortestTitle – the shortest book title found in the collection.
 * maxPrice – the highest book price in the collection.
 * maxTitle – the title of the book with the highest price.
 * minPrice – the lowest book price in the collection.
 * minTitle – the title of the book with the lowest price.
 * count1Star – the number of books with a 1-star rating.
 * avgPrice – the average price of all books in the collection.
 *
 * The function iterates through all books and computes the required statistics.
 */
void calculateStatisticsSerial(
    const std::vector<BookInfo>& books,
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
    count5Stars = 0;
    count1Star = 0;
    maxPrice = -1;
    minPrice = 1e9;
    double sum = 0.0;
    size_t max_len = 0, min_len = std::numeric_limits<size_t>::max();
    longestTitle.clear();
    shortestTitle.clear();
    maxTitle.clear();
    minTitle.clear();

    for (const auto& book : books) {
        if (book.rating == 5)
            count5Stars++;
        if (book.rating == 1)
            count1Star++;
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

    avgPrice = books.size() ? (sum / books.size()) : 0.0;
}


/**
 * Writes the calculated book statistics and results to a text file ("WebScraperSerial.txt").
 *
 * books - Vector of all BookInfo objects (books) that were processed.
 * count5Stars - Number of books with a 5-star rating.
 * longestTitle - The longest book title found.
 * shortestTitle - The shortest book title found.
 * numPages - Number of downloaded pages.
 * numUniqueUrls - Number of unique URLs that were visited.
 * seconds - Total time taken for the operation (in seconds).
 * throughput - Throughput in pages per second.
 * avgPrice - Average book price.
 * maxTitle - Title of the most expensive book.
 * maxPrice - Price of the most expensive book.
 * minTitle - Title of the cheapest book.
 * minPrice - Price of the cheapest book.
 * count1Star - Number of books with a 1-star rating.
 *
 * The function outputs all statistics in a human-readable format to the file.
 */
void writeResultsSerial(
    const std::vector<BookInfo>& books,
    int count5Stars,
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
    std::ofstream out("WebScraperSerial.txt");
    out << "Total number of books: " << books.size() << "\n";
    out << "Number of books with 5 stars: " << count5Stars << "\n";
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


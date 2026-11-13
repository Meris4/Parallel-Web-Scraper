#include "HtmlParser.hpp"
#include "Downloader.hpp"
#include <gumbo.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/parallel_for.h>
#include <queue>


/**
 * Recursively extracts all text content from the given GumboNode and its children.
 *
 * node - Pointer to the GumboNode to extract text from.
 * @return A std::string containing the concatenated text content.
 */
std::string getText(const GumboNode* node) {
    if (node->type == GUMBO_NODE_TEXT) {
        return node->v.text.text;
    }
    else if (node->type == GUMBO_NODE_ELEMENT || node->type == GUMBO_NODE_DOCUMENT) {
        std::string text;
        const GumboVector* children = &node->v.element.children;
        for (size_t i = 0; i < children->length; ++i) {
            text += getText(static_cast<GumboNode*>(children->data[i]));
        }
        return text;
    }
    return "";
}

/**
 * Recursively searches for a <p> element with class containing "price_color".
 *
 * node - Pointer to the GumboNode to search within.
 * @return Pointer to the found GumboNode representing the price <p> element, or nullptr if not found.
 */
const GumboNode* findPriceP(const GumboNode* node) {
    if (node->type == GUMBO_NODE_ELEMENT && node->v.element.tag == GUMBO_TAG_P) {
        GumboAttribute* class_attr = gumbo_get_attribute(&node->v.element.attributes, "class");
        if (class_attr && std::string(class_attr->value).find("price_color") != std::string::npos) {
            return node;
        }
    }
    if (node->type == GUMBO_NODE_ELEMENT || node->type == GUMBO_NODE_DOCUMENT) {
        const GumboVector* children = &node->v.element.children;
        for (size_t i = 0; i < children->length; ++i) {
            const GumboNode* found = findPriceP(static_cast<const GumboNode*>(children->data[i]));
            if (found) return found;
        }
    }
    return nullptr;
}

/**
 * Recursively searches for a <p> element with class containing "star-rating".
 *
 * node - Pointer to the GumboNode to search within.
 * @return Pointer to the found GumboNode representing the rating <p> element, or nullptr if not found.
 */
const GumboNode* findRatingP(const GumboNode* node) {
    if (node->type == GUMBO_NODE_ELEMENT && node->v.element.tag == GUMBO_TAG_P) {
        GumboAttribute* class_attr = gumbo_get_attribute(&node->v.element.attributes, "class");
        if (class_attr && std::string(class_attr->value).find("star-rating") != std::string::npos) {
            return node;
        }
    }
    if (node->type == GUMBO_NODE_ELEMENT || node->type == GUMBO_NODE_DOCUMENT) {
        const GumboVector* children = &node->v.element.children;
        for (size_t i = 0; i < children->length; ++i) {
            const GumboNode* found = findRatingP(static_cast<const GumboNode*>(children->data[i]));
            if (found) return found;
        }
    }
    return nullptr;
}

/**
 * Recursively parses a GumboNode tree to extract book information from <article class="product_pod"> elements.
 *
 * node - Pointer to the GumboNode to parse.
 * books - Reference to a vector where found BookInfo objects will be appended.
 * @return void
 */
void parseBookArticle(const GumboNode* node, std::vector<BookInfo>& books) {
    if (node->type != GUMBO_NODE_ELEMENT) return;

    if (node->v.element.tag == GUMBO_TAG_ARTICLE) {
        GumboAttribute* class_attr = gumbo_get_attribute(&node->v.element.attributes, "class");
        if (class_attr && std::string(class_attr->value) == "product_pod") {
            BookInfo book;
            const GumboVector* children = &node->v.element.children;
            for (size_t i = 0; i < children->length; ++i) {
                const GumboNode* child = static_cast<const GumboNode*>(children->data[i]);
                if (child->type == GUMBO_NODE_ELEMENT && child->v.element.tag == GUMBO_TAG_H3) {
                    const GumboVector* h3_children = &child->v.element.children;
                    for (size_t j = 0; j < h3_children->length; ++j) {
                        const GumboNode* a_node = static_cast<const GumboNode*>(h3_children->data[j]);
                        if (a_node->type == GUMBO_NODE_ELEMENT && a_node->v.element.tag == GUMBO_TAG_A) {
                            GumboAttribute* title_attr = gumbo_get_attribute(&a_node->v.element.attributes, "title");
                            if (title_attr) {
                                book.title = title_attr->value;
                            }
                        }
                    }
                }
                const GumboNode* price_p = findPriceP(node);
                if (price_p) {
                    std::string price_text = getText(price_p);
                    book.currency.clear();
                    std::string number;
                    for (char c : price_text) {
                        if (!isdigit(c) && c != '.' && !isspace(c)) {
                            book.currency = c;
                        }
                        if ((c >= '0' && c <= '9') || c == '.') {
                            number += c;
                        }
                    }
                    try {
                        book.price = number.empty() ? 0.0 : std::stod(number);
                    }
                    catch (...) {
                        book.price = 0.0;
                    }
                }
            }
            const GumboNode* rating_p = findRatingP(node);
            if (rating_p) {
                GumboAttribute* class_attr = gumbo_get_attribute(&rating_p->v.element.attributes, "class");
                if (class_attr) {
                    std::string rating_class = class_attr->value;
                    if (rating_class.find("One") != std::string::npos) book.rating = 1;
                    else if (rating_class.find("Two") != std::string::npos) book.rating = 2;
                    else if (rating_class.find("Three") != std::string::npos) book.rating = 3;
                    else if (rating_class.find("Four") != std::string::npos) book.rating = 4;
                    else if (rating_class.find("Five") != std::string::npos) book.rating = 5;
                    else book.rating = 0;
                }
            }
            else {
                book.rating = 0;
            }
            books.push_back(book);
        }
    }
    if (node->type == GUMBO_NODE_ELEMENT || node->type == GUMBO_NODE_DOCUMENT) {
        const GumboVector* children = &node->v.element.children;
        for (size_t i = 0; i < children->length; ++i) {
            parseBookArticle(static_cast<const GumboNode*>(children->data[i]), books);
        }
    }
}

/**
 * Parses the HTML content of a page and extracts all books into the provided vector.
 *
 * html - The HTML content as a std::string.
 * books - Reference to a vector where found BookInfo objects will be appended.
 * @return void
 */
void parseBooks(const std::string& html, std::vector<BookInfo>& books) {
    GumboOutput* output = gumbo_parse(html.c_str());
    parseBookArticle(output->root, books);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
}

/**
 * Recursively searches for pagination links (hrefs containing "page-") in the GumboNode tree.
 *
 * node - Pointer to the GumboNode to search within.
 * links - Reference to a vector where found pagination links will be appended.
 * @return void
 */
void findPaginationLinks(const GumboNode* node, std::vector<std::string>& links) {
    if (node->type == GUMBO_NODE_ELEMENT && node->v.element.tag == GUMBO_TAG_A) {
        GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (href) {
            std::string link = href->value;
            if (link.find("page-") != std::string::npos) {
                links.push_back(link);
            }
        }
    }
    if (node->type == GUMBO_NODE_ELEMENT || node->type == GUMBO_NODE_DOCUMENT) {
        const GumboVector* children = &node->v.element.children;
        for (size_t i = 0; i < children->length; ++i) {
            findPaginationLinks(static_cast<const GumboNode*>(children->data[i]), links);
        }
    }
}

/**
 * Sequentially crawls the site starting from the given URL, discovering all pagination links and collecting their URLs.
 *
 * start_url - The starting URL for crawling.
 * foundUrls - Reference to a concurrent vector where discovered URLs will be stored.
 * @return void
 */
void parallelIndexSite(
    const std::string& start_url,
    tbb::concurrent_vector<std::string>& foundUrls
) {
    tbb::concurrent_hash_map<std::string, bool> visited;
    std::queue<std::string> to_visit;

    to_visit.push(start_url);

    while (!to_visit.empty()) {
        std::string current_url;
        
        if (to_visit.empty()) break;
        current_url = to_visit.front();
        to_visit.pop();
        
        if (visited.count(current_url)) continue;
        visited.insert({ current_url, true });
        foundUrls.push_back(current_url);

        std::string html = fetchUrl(current_url, 10, 3);
        GumboOutput* output = gumbo_parse(html.c_str());
        const GumboNode* root = output->root;

        // Recursively search all pagination links
        std::vector<std::string> links;
        findPaginationLinks(root, links);
        for (const auto& link : links) {
            std::string full_url = "http://books.toscrape.com/catalogue/" + link;
            if (!visited.count(full_url)) {
                to_visit.push(full_url);
            }
        }
        gumbo_destroy_output(&kGumboDefaultOptions, output);
    }
}

/**
 * Parses all HTML pages in parallel and extracts book information into a concurrent vector.
 *
 * htmlPages - A concurrent vector of HTML page contents to parse.
 * books - A concurrent vector where all found BookInfo objects will be stored.
 * @return void
 */
void parseAllPages(
    const tbb::concurrent_vector<std::string>& htmlPages,
    tbb::concurrent_vector<BookInfo>& books
) {
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, htmlPages.size()), 
        [&](const tbb::blocked_range<size_t>& r) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                std::vector<BookInfo> local_books;
                parseBooks(htmlPages[i], local_books);
                for (const auto& book : local_books) {
                    books.push_back(book);
                }
            }
        }
    );
}
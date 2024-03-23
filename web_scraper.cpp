#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <future>
#include <string>
#include <mutex>

#include <curl/curl.h>
#include <htmlcxx/html/ParserDom.h>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    s->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string scrape_webpage(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string html_content;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html_content);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    htmlcxx::HTML::ParserDom parser;
    tree<htmlcxx::HTML::Node> dom = parser.parseTree(html_content);

    return "Scraped data from " + url;
}

void worker(std::queue<std::string>& urls, std::mutex& mtx, std::vector<std::string>& results) {
    while (true) {
        mtx.lock();
        if (urls.empty()) {
            mtx.unlock();
            break;
        }
        std::string url = urls.front();
        urls.pop();
        mtx.unlock();

        std::string result = scrape_webpage(url);
        mtx.lock();
        results.push_back(result);
        mtx.unlock();
    }
}

void traverse_dom(const tree<htmlcxx::HTML::Node>& dom, tree<htmlcxx::HTML::Node>::iterator it) {
    for (; it != dom.end(); ++it) {
        // Здесь вы можете обрабатывать узел DOM
        std::string tag_name = it->tagName();
        std::string text = it->text();

        std::cout << "Tag: " << tag_name << ", Text: " << text << std::endl;

        // Если у узла есть дочерние элементы, функция будет рекурсивно вызвана для их обработки
        if (!dom.empty(it)) {
            traverse_dom(dom, dom.begin(it));
        }
    }
}

int main() {
    std::queue<std::string> urls;
    std::vector<std::string> results;
    std::mutex mtx;

    urls.push("http://example.com/page1");
    urls.push("http://example.com/page2");
    urls.push("http://example.com/page3");

    // Запускаем потоки
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 3; ++i) {
        futures.push_back(std::async(std::launch::async, worker, std::ref(urls), std::ref(mtx), std::ref(results)));
    }

    // Ожидаем завершения всех рабочих потоков
    for (auto& future : futures) {
        future.get();
    }

    // Вывод
    for (const auto& result : results) {
        std::cout << result << std::endl;
    }

    return 0;
}

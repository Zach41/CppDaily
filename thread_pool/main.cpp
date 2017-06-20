#include <iostream>
#include <vector>
#include <chrono>
#include <string>

#include "threadpool.h"

int main(int, char **)
{
    pool::ThreadPool pool(4);
    std::vector<std::future<int>> results;

    for (auto i = 0; i < 8; i++)
    {
        results.emplace_back(
            pool.enqueue([i]() -> int {
                auto out_str = "Hello " + std::to_string(i) + '\n';
                std::cout << out_str;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                out_str = "World " + std::to_string(i) + '\n';
                std::cout << out_str;
                return i * i;
            }));
    }

    for (auto &&result : results)
        std::cout << result.get() << ' ';
    std::cout << std::endl;
    return 0;
}
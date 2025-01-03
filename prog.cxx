#include "ActProgressBar.h"

#include <chrono>
#include <iostream>
#include <ostream>
#include <set>
#include <thread>
#include <vector>
void prog()
{

    // std::vector<std::set<int>> runs {{1, 2, 3}, {4, 5, 6}, {7, 8}, {9, 10, 11, 12, 13}};
    std::vector<std::set<int>> runs {{1, 2, 3}};
    ActRoot::ProgressBar p {};
    p.SetNThreads(runs.size());

    auto worker {[&](unsigned int i)
                 {
                     int count {1};
                     for(const auto& run : runs[i])
                     {
                         auto entries {100};
                         p.SetThreadInfo(i, entries, runs[i].size());
                         for(auto e = 0; e < entries; e++)
                         {
                             p.SetThreadStatus(i, e, entries, run, count);
                             std::this_thread::sleep_for(std::chrono::milliseconds(1));
                         }
                         count++;
                     }
                     p.IncrementCompleted();
                 }};

    p.Init();
    std::vector<std::thread> threads;
    for(unsigned int i = 0; i < runs.size(); i++)
        threads.emplace_back(worker, i);
    for(auto& thread : threads)
        thread.join();
    p.Join();
}

int a()
{
    // Print 4 lines
    for(int i = 0; i < 4; ++i)
    {
        std::cout << "Line " << (i + 1) << std::endl;
    }

    // Wait a second
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Overwrite each line one by one, move the cursor up and clear it
    std::cout << "\033[4A";
    for(int i = 0; i < 4; ++i)
    {
        std::cout << "\033[K"; // Clear the current line
        std::cout << "Updated Line " << (i + 1) << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Pause to see updates
        std::cout << std::flush;                              // Ensure immediate output
    }
    a();
    return 0;
}

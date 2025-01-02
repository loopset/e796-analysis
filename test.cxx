#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <vector>
class Test
{
private:
    int fNThreads {};
    int fRuns {100};
    int fEntries {1000};
    std::vector<std::pair<long int, int>> fThreadInfo {};
    std::vector<std::pair<int, int>> fThreadProgress {};
    std::vector<std::thread> fThreads {};
    std::thread fPrintThread {};
    std::mutex fMutex {};

public:
    Test(int n) : fNThreads(n), fThreadProgress(n, {0, 0}), fThreadInfo(n, {0, 0}) {}
    // Start the progress manager
    void start()
    {
        // Clear screen and hide cursor
        // std::cout << "\033[2J\033[?25l";
        std::cout << "Start " << '\n';
        for(int i = 0; i < fNThreads; i++)
        {
            fThreadInfo[i] = {fEntries, fRuns};
            std::cout << '\n';
        }

        // Launch the printer thread
        fPrintThread = std::thread(&Test::print_progress_bars, this);

        // Launch worker threads
        for(int i = 0; i < fNThreads; ++i)
        {
            fThreads.emplace_back(&Test::worker_task, this, i);
        }
    }

    // Wait for all threads to complete
    void join()
    {
        // Wait for all worker threads to finish
        for(auto& worker : fThreads)
        {
            worker.join();
        }

        // Wait for the printer thread to finish
        fPrintThread.join();

        // Show cursor again and move to a new line
        std::cout << "\033[?25h\n";
    }
    // Print progress bars in the terminal
    void print_progress_bars()
    {
        while(true)
        {
            {
                std::lock_guard<std::mutex> lock {fMutex};
                std::cout << "\033[" << fNThreads - 1 << "H"; // Move cursor to the top of the terminal

                // Print progress bars for each thread
                for(int i = 0; i < fNThreads; ++i)
                {
                    int bar_width = 50;
                    int pos = bar_width * fThreadProgress[i].first / fEntries;
                    std::cout << "Thread " << i << ": [";
                    for(int j = 0; j < bar_width; ++j)
                    {
                        if(j < pos)
                            std::cout << "=";
                        else if(j == pos)
                            std::cout << ">";
                        else
                            std::cout << " ";
                    }
                    std::cout << "] " << fThreadProgress[i].second << " of " << fThreadInfo[i].second << "\n";
                }
            }

            // Check if all threads are done
            if(all_done())
                break;

            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Update interval
        }
    }

    // Check if all worker threads have completed their tasks
    bool all_done()
    {
        for(auto p : fThreadProgress)
        {
            if(p.second < 100)
                return false;
        }
        return true;
    }

    // Worker task to simulate work and update progress
    void worker_task(int thread_id)
    {
        for(int i = 0; i <= 100; ++i)
        {
            for(int j = 0; j < 1000; j++)
            {
                {
                    std::lock_guard<std::mutex> lock {fMutex};
                    fThreadProgress[thread_id] = {j, i};                           // Update this thread's progress
                    std::this_thread::sleep_for(std::chrono::nanoseconds(1)); // Simulate work
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Simulate work
        }
    }
};

void test()
{
    Test obj {5}; // Create progress manager
    obj.start();  // Start progress management
    obj.join();   // Wait for all threads to complete
}

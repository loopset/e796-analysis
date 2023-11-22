#include "TSystem.h"
#include <thread>
#include <vector>

void thing(int i)
{
    gSystem->Sleep(1);
}

void testMT()
{
    std::vector<std::thread> threads;
    for(int i = 0; i < 8; i++)
        threads.push_back(std::thread(thing, i));

    for(auto& t : threads)
    {
        if(t.joinable())
            t.join();
    }

}

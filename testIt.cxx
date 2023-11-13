#include "ActInputData.h"
#include "ActInputIterator.h"

#include <iostream>

void Print(ActRoot::InputIterator& it)
{
    auto [run, entry] {it.GetCurrentRunEntry()};
    std::cout << "Run : " << run << " entry : " << entry << '\n';
}

void testIt()
{
    ActRoot::InputData input {"./configs/cluster.runs"};
    const auto& manual {input.GetManualEntries()};

    ActRoot::InputIterator it {&input};
    Print(it);
    it.Next();
    Print(it);
    it.Next();
    Print(it);
    it.Next();
    Print(it);
    it.Previous();
    Print(it);
}

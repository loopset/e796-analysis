#include "TString.h"
#include "TSystem.h"

#include <thread>
#include <vector>
void RunAll()
{
    auto worker {[](TString which)
                 {
                     // Get histos
                     gSystem->Exec(TString::Format("root -l -b -x -q 'DoHists.cxx(\"%s\")'", which.Data()));
                     // Do fits
                     gSystem->Exec(TString::Format("root -l -b -x -q 'DoFits.cxx(\"%s\")'", which.Data()));
                 }};
    std::vector<std::thread> threads;
    for(const auto& mode : {"side", "veto", "antiveto"})
        threads.emplace_back(worker, mode);
    for(auto& t : threads)
        t.join();
}

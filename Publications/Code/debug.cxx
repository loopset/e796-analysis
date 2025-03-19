#include "ActDataManager.h"
#include "ActTPCData.h"
#include "ActTypes.h"

#include <memory>
#include <string>

void Write(ActRoot::TPCData* data, const std::string& file)
{
    auto f {std::make_unique<TFile>(file.c_str(), "recreate")};
    f->WriteObject(data, "TPCData");
}

void debug()
{
    int run {156};
    int entry {8742};

    ActRoot::DataManager datman {"./../../configs/data.conf", ActRoot::ModeType::EFilter};
    datman.SetRuns(run, run);
    auto chain {datman.GetChain()};

    ActRoot::TPCData* data {};
    chain->SetBranchAddress("TPCData", &data);

    chain->GetEntry(entry);
    data->Print();
    Write(data, "./Events/debug_small_chi.root");
    // for(auto& cl : data->fClusters)
    // {
    //     cl.ReFit();
    //     cl.Print();
    //     cl.GetLine().Print();
    // }
    // Write(data, "./Events/debug_NO_qweight.root");
}

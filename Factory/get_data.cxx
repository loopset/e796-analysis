#include "ActDataManager.h"
#include "ActTPCData.h"
#include "ActTypes.h"

#include <memory>

void write(const std::string& file, ActRoot::TPCData* ptr)
{
    auto f {std::make_unique<TFile>(file.c_str(), "recreate")};
    f->WriteObject(ptr, "TPCData");
}

void get_data()
{
    ActRoot::DataManager datman {"../configs/data.conf", ActRoot::ModeType::EReadTPC};
    datman.SetRuns(155, 155);
    auto chain {datman.GetChain()};

    ActRoot::TPCData* data {};
    chain->SetBranchAddress("TPCData", &data);

    // Get entry and save it in file
    int entry {14};
    chain->GetEntry(entry);
    write("./Inputs/3body.root", data);
}

#include "ActDataManager.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

#include "TROOT.h"

#include "../Gates.cxx"
#include "../HistConfig.h"
#include "../Utils.cxx"

void Pipe0_Beam()
{
    ROOT::EnableImplicitMT();
    // Read data
    ActRoot::DataManager datman {"/media/Data/E796v2/configs/data.conf", ActRoot::ModeType::EReadSilMod};
    auto chain {datman.GetJoinedData()};
    ROOT::RDataFrame df {*chain};
    df.Describe().Print();

    // Get CFA values
}

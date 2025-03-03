#include "ActDataManager.h"
#include "ActSilData.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include <string>
#include <vector>

#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
void PIDTwoSils()
{
    ROOT::EnableImplicitMT();
    // Read data
    ActRoot::DataManager datman {"../../configs/data.conf", ActRoot::ModeType::EReadSilMod};
    auto chain {datman.GetChain()};
    ROOT::RDataFrame df {*chain};
    // Filter
    auto twosils {df.Filter(
        [](ActRoot::SilData& data)
        {
            if(data.fSiN.size() == 2)
            {
                if(data.fSiN.count("f0") && data.fSiN.count("f1"))
                    return true;
            }
            return false;
        },
        {"SilData"})};

    auto def = twosils
                   .Define("Layers",
                           [](ActRoot::SilData& data)
                           {
                               std::vector<std::string> ret;
                               for(const auto& [key, vals] : data.fSiN)
                                   for(const auto& val : vals)
                                       ret.push_back(key);
                               return ret;
                           },
                           {"SilData"})
                   .Define("Energies",
                           [](ActRoot::SilData& data)
                           {
                               std::vector<float> ret;
                               for(const auto& [key, vals] : data.fSiE)
                                   for(const auto& val : vals)
                                       ret.push_back(val);
                               return ret;
                           },
                           {"SilData"});

    def.Snapshot("Simple_Tree", "./twopid.root", {"Layers", "Energies"});
    // Get PID
    // auto hPID {twosils.Define("ESil0", "fSilEs[0]")
    //                .Define("ESil1", "fSilEs[1]")
    //                .Histo2D(HistConfig::PIDTwo, "ESil1", "ESil0")};
    //
    // // Get cuts
    // // ActRoot::CutsManager<int> cut;
    // // cut.ReadCut(0, "./debug_hes.root");
    // // std::ofstream streamer {"./debug_hes_twosils.dat"};
    // // twosils.Foreach(
    // //     [&](const ActRoot::MergerData& d)
    // //     {
    // //         if(cut.IsInside(0, d.fSilEs[1], d.fSilEs[0]))
    // //             streamer << d.fRun << " " << d.fEntry << '\n';
    // //     },
    // //     {"MergerData"});
    // // streamer.close();
    //
    // // plot
    // auto* c1 {new TCanvas("c1", "Two sils PID")};
    // hPID->DrawClone("colz");
}

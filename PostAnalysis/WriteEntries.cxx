#ifndef WriteEntries_cxx
#define WriteEntries_cxx

#include "ActCutsManager.h"
#include "ActDataManager.h"
#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"

#include <fstream>
#include <string>

#include "/media/Data/E796v2/PostAnalysis/Utils.cxx"

void WriteEntries(const std::string& beam, const std::string& target, const std::string& light, bool isSide)
{
    ActRoot::DataManager datman {"../configs/data.conf"};
    auto chain {datman.GetJoinedData()};
    ROOT::RDataFrame df {*chain};

    // ROOT::RDataFrame d {"Final_Tree", E796Utils::GetFileName(2, beam, target, light, isSide)};
    //
    // // Apply any filter function
    // // auto df {d};
    // // Read cut
    // ActRoot::CutsManager<int> cut;
    // cut.ReadCut(0, "./Cuts/p_d_2.root");
    // auto df {d.Filter([&](double evertex, float thetalab) { return cut.IsInside(0, thetalab, evertex); },
    //                   {"EVertex", "fThetaLight"})};
    // // auto df {d.Filter("fThetaBeam > 1.5")}; // in deg
    //
    // Write to file
    std::ofstream streamer {
        TString::Format("./Entries/entries_%s_%s_%s_befPileUp.dat", beam.c_str(), target.c_str(), light.c_str())};
    df.Foreach([&](const ActRoot::MergerData& data) { streamer << data.fRun << " " << data.fEntry << '\n'; },
               {"MergerData"});
    streamer.close();
    //
    // // df.Snapshot("dt", "/media/Data/E796v2/PostAnalysis/Cuts/20O_d_t.root",
    // //             {"Ex", "fThetaLight", "EBeam", "EVertex", "fEntry", "fRun"});
    //
    std::cout << "Written " << df.Count().GetValue() << " entries to file!" << '\n';
}
#endif

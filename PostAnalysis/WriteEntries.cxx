#ifndef WriteEntries_cxx
#define WriteEntries_cxx

#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"

#include <fstream>
#include <string>

#include "/media/Data/E796v2/PostAnalysis/Utils.cxx"

void WriteEntries(const std::string& beam, const std::string& target, const std::string& light, bool isSide)
{
    ROOT::RDataFrame d {"Final_Tree", E796Utils::GetFileName(2, beam, target, light, isSide)};
    
    // Apply any filter function
    auto df {d.Filter("fThetaBeam < 0.25 && 20 <= fRP.fCoordinates.fX && fRP.fCoordinates.fX <= 40")};// in deg

    // Write to file
    std::ofstream streamer {TString::Format("/media/Data/E796v2/PostAnalysis/Cuts/entries_%s_%s_%s.dat", beam.c_str(),
                                            target.c_str(), light.c_str())};
    df.Foreach([&](const ActRoot::MergerData& data) { streamer << data.fRun << " " << data.fEntry << '\n'; },
               {"MergerData"});
    streamer.close();

    std::cout << "Written " << df.Count().GetValue() << " entries to file!" << '\n';
}
#endif

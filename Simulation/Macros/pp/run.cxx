#include "TString.h"
#include "TSystem.h"

#include <vector>
void run()
{
    return;// disabled jsut for safety
    // Vary point
    std::vector<double> points {};
    for(double y = 80; y <= 120; y += 10)
        points.push_back((256. + y) / 2);
    points.push_back(174.425); // original state

    TString pwd {gSystem->pwd()};
    for(const auto& y : points)
    {
        // Go to sim dir
        gSystem->cd("/media/Data/E796v2/Simulation/");
        gSystem->Exec(TString::Format("gawk -i inplace 'BEGIN {found=0} /specs->GetLayer\\(\"l0\"\\)\\.SetPoint\\(/ "
                                      "&& found==0 { sub(/\\{0,[^,]*, 0\\}/, "
                                      "\"{0, %.3f, 0}\"); found=1} {print}' Simulation_E796.cpp",
                                      y));
        // Run!
        gSystem->Exec("root -l -b -x -q 'Runner.cxx(\"simu\", false)'");
        // Go to ang dir
        gSystem->cd("/media/Data/E796v2/Fits/pp");
        gSystem->Exec("root -l -b -x -q Ang.cxx");
        // Copy and save figures
        gSystem->cd("Outputs/");
        auto dir {TString::Format("ypoint_%.1f", y)};
        gSystem->mkdir(dir);
        gSystem->Exec(TString::Format("cp effs.png %s/effs.png", dir.Data()));
        gSystem->Exec(TString::Format("cp ang_0.png %s/ang_0.png", dir.Data()));
    }
    gSystem->cd(pwd);

    // // Vary beam offset
    // std::vector<double> offsets {};
    // for(double o = 6; o <= 8; o += 0.5)
    //     offsets.push_back(o);
    // offsets.push_back(7.52); // determined from emittance. back to previous state
    //
    // TString pwd {gSystem->pwd()};
    // for(const auto& off : offsets)
    // {
    //     // Go to sim dir
    //     gSystem->cd("/media/Data/E796v2/Simulation/");
    //     // Modify offset ONLY FOR FIRST appereance
    //     gSystem->Exec(TString::Format(
    //         "gawk -i inplace 'BEGIN {found=0}  /^[ \t]*beamOffset *=/ && found==0 {sub(/=.*;/, \" = %.2f;\"); "
    //         "found=1}  {print}' Simulation_E796.cpp",
    //         off));
    //     // Run!
    //     gSystem->Exec("root -l -b -x -q 'Runner.cxx(\"simu\", false)'");
    //     // Go to ang dir
    //     gSystem->cd("/media/Data/E796v2/Fits/pp");
    //     gSystem->Exec("root -l -b -x -q Ang.cxx");
    //     // Copy and save figures
    //     gSystem->cd("Outputs/");
    //     auto dir {TString::Format("offset_%.1f", off)};
    //     gSystem->mkdir(dir);
    //     gSystem->Exec(TString::Format("cp effs.png %s/effs.png", dir.Data()));
    //     gSystem->Exec(TString::Format("cp ang_0.png %s/ang_0.png", dir.Data()));
    //     // gSystem->CopyFile("effs.png", dir);
    // }
    // gSystem->cd(pwd);
}

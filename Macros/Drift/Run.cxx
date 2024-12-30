#include "TString.h"
#include "TSystem.h"

#include <iostream>
#include <vector>
void Run()
{
    TString pwd {gSystem->pwd()};
    // 0-> Define drift velocities
    double conv {1. / 4 / 0.08};
    std::vector<double> drifts;
    for(double d = 2.3; d <= 2.65; d += 0.05)
    {
        drifts.push_back(d);
        std::cout << "vdrift : " << d * conv << '\n';
    }
    drifts.push_back(2.384); // to leave everything where we started

    for(const auto& drift : drifts)
    {
        gSystem->cd(pwd);
        std::cout << "Running for vdrift : " << drift * conv << '\n';
        // 1-> Set drift in detector config
        gSystem->Exec(
            TString::Format("sed -i '/^DriftFactor/s/:.*$/: %.3f/' /media/Data/E796v2/configs/detector.conf", drift));
        // 2-> Exec actroot
        gSystem->cd("/media/Data/E796v2/");
        gSystem->Exec("actroot -m && actroot -c");
        // 3-> Generate antiveto matrix
        gSystem->cd("/media/Data/E796v2/Macros/SilVetos/");
        gSystem->Exec("root -l -b -x -q 'DoHists.cxx(\"antiveto\")' && root -l -b -x -q 'DoFits.cxx(\"antiveto\")'");
        // Selector
        int idx {};
        for(int i = 0; i < 2; i++)
        {
            if(i == 0) //(p,d)
            {
                gSystem->Exec("sed -i '/^Target/s/:.*$/: p/' /media/Data/E796v2/Selector/selector.conf");
                gSystem->Exec("sed -i '/^Light/s/:.*$/: d/' /media/Data/E796v2/Selector/selector.conf");
            }
            else //(d,t)
            {
                gSystem->Exec("sed -i '/^Target/s/:.*$/: d/' /media/Data/E796v2/Selector/selector.conf");
                gSystem->Exec("sed -i '/^Light/s/:.*$/: t/' /media/Data/E796v2/Selector/selector.conf");
            }
            idx++;
            // Execute Pipelines
            gSystem->cd("/media/Data/E796v2/PostAnalysis/");
            gSystem->Exec("root -l -b -x -q 'Runner.cxx(\"123\")'");
            // And finally ComputeProfiles
            gSystem->cd("/media/Data/E796v2/Macros/Drift/");
            gSystem->Exec(TString::Format("root -l -b -x -q 'GetProfile.cxx(%.3f)'", drift));
        }
    }
}

#include "TString.h"
#include "TSystem.h"


void run()
{
    TString pwd {gSystem->pwd()};

    int niter {2};
    for(int i = 0; i < niter; i++)
    {
        auto tag {TString::Format("sys_angle_%d", i)};
        // Modify selector.conf
        gSystem->cd("/media/Data/E796v2/Selector/");
        gSystem->Exec(
            TString::Format("gawk -i inplace '/^Flag: /{print; print \"Tag: %s\"; next}1' selector.conf", tag.Data()));

        gSystem->cd("/media/Data/E796v2/Simulation/");
        gSystem->Exec("root -l -b -x -q 'Runner.cxx(\"simu\", false)'");

        // Delete any tag after iteration finishes
        gSystem->cd("/media/Data/E796v2/Selector/");
        gSystem->Exec("gawk -i inplace '!/^Tag:/' selector.conf");
    }
    gSystem->cd(pwd);
}

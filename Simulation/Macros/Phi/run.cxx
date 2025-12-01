#include "TString.h"
#include "TSystem.h"

#include <string>
#include <vector>

void run()
{
    std::vector<TString> channels {"gawk -i inplace '/^Target:/ { $2=\"p\" } /^Light:/ { $2=\"p\" }1' selector.conf",
                                   "gawk -i inplace '/^Target:/ { $2=\"d\" } /^Light:/ { $2=\"d\" }1' selector.conf"};
    std::vector<std::string> flags {"phi_low", "phi_middle", "phi_up"};
    for(const auto& channel : channels)
    {
        for(const auto& flag : flags)
        {
            // Modify selector.conf
            gSystem->cd("/media/Data/E796v2/Selector/");
            gSystem->Exec(channel);
            gSystem->Exec(
                TString::Format("gawk -i inplace '/^Flag:/ { sub(/:.*/, \": %s\") }1' selector.conf", flag.c_str()));

            // Run Runner
            gSystem->cd("/media/Data/E796v2/Simulation/");
            gSystem->Exec("root -l -b -x -q 'Runner.cxx(\"simu\", false)'");

            // Back to default setting
            gSystem->cd("/media/Data/E796v2/Selector/");
            gSystem->Exec("gawk -i inplace '/^Flag:/ { sub(/:.*/, \": juan_RPx\") }1' selector.conf");
        }
    }
}

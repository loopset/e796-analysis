#include "TSystem.h"

#include <string>
#include <vector>
void all()
{
    std::string pwd {gSystem->pwd()};
    std::vector<std::string> paths {"pp", "dd", "pd", "dt"};
    for(const auto& path : paths)
    {
        gSystem->cd(pwd.c_str());
        gSystem->cd(path.c_str());
        gSystem->Exec("root -l -b -x -q Ang.cxx");
    }
}

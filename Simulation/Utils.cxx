#ifndef E796Simu_Utils_cxx
#define E796Simu_Utils_cxx
#include "TString.h"

#include <string>
namespace E796Simu
{
    TString GetFile(const std::string& beam, const std::string& target, const std::string& light, double Eex,
                    int nPS = 0, int pPS = 0)
    {
        return TString::Format(
            "/media/Data/E796v2/Simulation/Outputs/e796_beam_%s_target_%s_light_%s_Eex_%.2f_nPS_%d_pPS_%d.root",
            beam.c_str(), target.c_str(), light.c_str(), Eex, nPS, pPS);
    }
} // namespace E796Simu
#endif

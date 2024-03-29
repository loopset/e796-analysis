#ifndef Utils_cxx
#define Utils_cxx

#include "ActColors.h"
#include "ActSilMatrix.h"

#include "TString.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace E796Utils
{
TString GetFileName(int pipe, const std::string& beam, const std::string& target, const std::string& light, bool isSide,
                    const std::string& type = "tree")
{
    auto path {TString::Format("/media/Data/E796v2/PostAnalysis/RootFiles/Pipe%d/", pipe)};
    TString name {};
    if(isSide)
        name = TString::Format("%s_beam_%s_target_%s_light_%s_side.root", type.c_str(), beam.c_str(), target.c_str(),
                               light.c_str());
    else
        name = TString::Format("%s_beam_%s_target_%s_light_%s_front.root", type.c_str(), beam.c_str(), target.c_str(),
                               light.c_str());
    return path + name;
}

ActPhysics::SilMatrix* GetVetoMatrix()
{
    auto* sm {new ActPhysics::SilMatrix {"Veto"}};
    sm->Read("/media/Data/E796v2/Macros/SilVetos/Outputs/veto_matrix.root");
    return sm;
}
ActPhysics::SilMatrix* GetAntiVetoMatrix()
{
    auto* sm {new ActPhysics::SilMatrix {"Antiveto"}};
    sm->Read("/media/Data/E796v2/Macros/SilVetos/Outputs/antiveto_matrix.root");
    return sm;
}

ActPhysics::SilMatrix* GetEffSilMatrix(const std::string& light)
{
    // if 3He or 4He we need all data -> bigger matrix
    if(light == "3He" || light == "4He")
    {
        std::cout << BOLDYELLOW << "E796Utils: Reading veto sil matrix for -> " << light << RESET << '\n';
        return GetVetoMatrix();
    }
    else if(light == "1H" || light == "2H" || light == "3H")
    {
        std::cout << BOLDYELLOW << "E796Utils: Reading antiveto sil matrix for -> " << light << RESET << '\n';
        return GetAntiVetoMatrix();
    }
    else
        throw std::runtime_error("E748Utils::GetEffSilMatrix(): not recognized light particle " + light);
}

std::string GetParticleName(const std::string& filename, const std::string& what)
{
    auto base {filename.find(what)};
    auto init {filename.find_first_of('_', base) + 1};
    auto end {filename.find_first_of('_', init)};
    return filename.substr(init, (end - init));
}

struct Signature
{
    std::string beam {};
    std::string target {};
    std::string light {};
    bool isSide {};
};

Signature ExtractSignature(const std::string& file)
{
    auto beam {GetParticleName(file, "beam")};
    auto target {GetParticleName(file, "target")};
    auto light {GetParticleName(file, "light")};
    auto side {TString(file).Contains("side")};
    return {beam, target, light, side};
}

} // namespace E796Utils
#endif // !Utils_cxx

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

ActPhysics::SilMatrix* GetSideMatrix()
{
    auto* sm {new ActPhysics::SilMatrix {"SideMatrix"}};
    sm->Read("/media/Data/E796v2/Macros/SilVetos/Outputs/side_matrix.root");
    return sm;
}

ActPhysics::SilMatrix* GetFrontSilMatrix(const std::string& light)
{
    // if 3He or 4He we need all data -> bigger matrix
    if(light == "3He" || light == "4He")
    // if(false)
    {
        std::cout << BOLDYELLOW << "E796Utils: Reading veto sil matrix for -> " << light << RESET << '\n';
        return GetVetoMatrix();
    }
    // else if(light == "1H" || light == "2H" || light == "3H" || light == "3He" || light == "4He")
    else if(light == "1H" || light == "2H" || light == "3H")
    {
        std::cout << BOLDYELLOW << "E796Utils: Reading antiveto sil matrix for -> " << light << RESET << '\n';
        return GetAntiVetoMatrix();
    }
    else
        throw std::runtime_error("E748Utils::GetEffSilMatrix(): not recognized light particle " + light);
}

ActPhysics::SilMatrix* GetEffSilMatrix(const std::string& target, const std::string& light)
{
    bool isEl {target == light};
    if(isEl)
    {
        std::cout << BOLDYELLOW << "E796Utils: Reading side sil matrix for -> " << light << RESET << '\n';
        return GetSideMatrix();
    }
    else
        return GetFrontSilMatrix(light);
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

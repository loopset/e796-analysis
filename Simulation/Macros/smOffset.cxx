#include "ActSilMatrix.h"

#include <string>

#include "../../PostAnalysis/Utils.cxx"

void smOffset()
{
    std::string light {"3He"};
    auto* sm {E796Utils::GetEffSilMatrix(light)};
    for(const auto& i : {3, 4})
    {
        auto* s {sm->GetSil(i)};
        double xy {};
        double z {};
        s->Center(xy, z);
        std::cout << "Sil " << i << " Z centre : " << z << '\n';
    }
}

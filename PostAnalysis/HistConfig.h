#ifndef HistConfig_h
#define HistConfig_h

#include "ROOT/RDF/HistoModels.hxx"

#include "TString.h"

namespace HistConfig
{
    using namespace ROOT::RDF;

    const TH2DModel PID {"hPID", "PID;E_{Sil} [MeV];Q_{ave} [mm^{-1}]", 300, 0, 40, 800, 0, 2000};

    const TH2DModel SP {"hSP", "SP;X or Y [mm];Z [mm]", 200, -10, 300, 200, -10, 300};

    const TH2DModel Kin {"hKin", "Kinematics;#theta_{Lab} [#circ];E_{Vertex} [MeV]", 500, 0, 180, 250, 0, 20};

    const TH1DModel Ex {"hEx", "Excitation energy;E_{x} [MeV];Counts", 200, -10, 20};

    template <typename T>
    T ChangeTitle(T model, const TString& title);
} // namespace HistConfig

template <typename T>
T HistConfig::ChangeTitle(T model, const TString& title)
{
    auto ret {model};
    TString old {model.fTitle};
    auto end {old.First(';')};
    TString nt {title + old(end, old.Length() - end)};
    ret.fTitle = nt;
    return ret;
}

#endif

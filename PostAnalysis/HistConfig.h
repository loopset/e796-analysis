#ifndef HistConfig_h
#define HistConfig_h
#include "ROOT/RDF/HistoModels.hxx"

namespace HistConfig
{
    using namespace ROOT::RDF;

    const TH2DModel PID {"hPID", "PID;E_{Sil} [MeV];Q_{ave} [mm^{-1}]", 200, 0, 50, 500, 0, 2000};

    const TH2DModel SP {"hSP", "SP;X or Y [mm];Z [mm]", 200, -10, 300, 200, -10, 300};
} // namespace HistConfig
#endif

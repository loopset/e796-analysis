#ifndef HistConfig_h
#define HistConfig_h

#include "ROOT/RDF/HistoModels.hxx"

#include "TString.h"

namespace HistConfig
{
using namespace ROOT::RDF;

const TH2DModel PID {"hPID", "PID;E_{Sil} [MeV];Q_{ave} [mm^{-1}]", 300, 0, 40, 800, 0, 2000};

const TH2DModel PIDTwo {"hPIDTwo", "PID with two silicons;E_{Si1} [MeV];E_{Si0} [MeV]", 500, 0, 40, 500, 0, 40};

const TH2DModel SP {"hSP", "SP;X or Y [mm];Z [mm]", 200, -10, 300, 200, -10, 300};

const TH2DModel RP {"hRP", "RP;X [mm];Y [mm]", 200, -10, 300, 200, -10, 300};

const TH1DModel TL {"hTL", "Track length; TL [mm]", 300, 0, 600};

const TH2DModel Kin {"hKin", "Kinematics;#theta_{Lab} [#circ];E_{Vertex} [MeV]", 250, 0, 60, 250, 0, 20};

const TH2DModel KinEl {"hKinEl", "Elastic kinematics;#theta_{Lab} [#circ];E_{Vertex} [MeV]", 600, 0, 180, 300, 0, 20};

const TH2DModel KinSimu {"hKin", "Simulation kinematics;#theta_{Lab} [#circ];E_{Vertex} [MeV]", 400, 0, 90, 300, 0, 40};

const TH2DModel KinCM {"hKinCM", "CM kinematics;#theta_{CM} [#circ];E_{Vertex} [MeV]", 400, 0, 60, 400, 0, 20};

const TH1DModel Ex {"hEx", "Excitation energy;E_{x} [MeV];Counts", 200, -10, 20};

const TH1DModel ThetaCM {"hThetaCM", "ThetaCM;#theta_{CM} [#circ]", 600, 0, 180};

const TH2DModel ZThetaZ {"hZThetaZ", "Emittance along Z;Z [mm];#theta_{Z} [#circ]", 600, 0, 270, 600, -10, 10};

const TH2DModel YPhiY {"hYPhiY", "Emittance along Y;Y [mm];#phi_{Y} [#circ]", 600, 0, 270, 600, -10, 10};

const TH2DModel ThetaBeam {
    "hThetaBeam", "#theta_{Beam} against RP.X;RP.X() [mm];#theta_{Beam} [#circ]", 200, -5, 270, 200, -1, 10};

const TH2DModel ExZ {"hExZ", "E_{x} dependence on SP.Z();SP.Z() [mm];E_{x} [MeV]", 200, -10, 300, 200, -10, 20};

const TH2DModel ExThetaCM {
    "hExThetaCM", "E_{x} vs #theta_{CM};#theta_{CM} [#circ];E_{x} [MeV]", 400, 0, 60, 200, -10, 20};

const TH2DModel ExThetaLab {
    "hExThetaLab", "E_{x} vs #theta_{Lab};#theta_{Lab} [#circ];E_{x} [MeV]", 400, 0, 60, 200, -10, 20};

const TH2DModel ExRPZ {"hExRPZ", "E_{x} vs RP.Z;RP.Z() [mm];E_{x} [MeV]", 200, -10, 300, 200, -10, 20};

const TH2DModel ThetaHeavyLight {
    "hThetaHL", "#theta heavy vs light;#theta_{Light} [#circ];#theta_{Heavy} [#circ]", 400, 0, 60, 400, 0, 60};

const TH2DModel ThetaCMLab {
    "hThetaCMLab", "CM vs Lab correlations;#theta_{Lab} [#circ];#theta_{CM} [#circ]", 400, 0, 60, 400, 0, 60};

const TH2DModel RPxThetaCM {
    "hRPxThetaCM", "RP.X vs #theta_{CM} correlations;RP.X [mm];#theta_{CM} [#circ]", 200, 0, 300, 100, 0, 60};

template <typename T>
T ChangeTitle(T model, const TString& title, const TString& label = "");
} // namespace HistConfig

template <typename T>
T HistConfig::ChangeTitle(T model, const TString& title, const TString& label)
{
    auto ret {model};
    if(label.Length() > 0)
        ret.fName = model.fName + label;
    TString old {model.fTitle};
    auto end {old.First(';')};
    TString nt {title + old(end, old.Length() - end)};
    ret.fTitle = nt;
    return ret;
}

#endif

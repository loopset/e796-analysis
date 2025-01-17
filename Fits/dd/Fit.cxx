#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TROOT.h"
#include "TString.h"
#include "TVirtualPad.h"

#include "FitInterface.h"
#include "FitModel.h"
#include "FitUtils.h"
#include "Interpolators.h"

#include <string>
#include <vector>

#include "../../Selector/Selector.h"
#include "/media/Data/E796v2/Fits/FitHist.h"
void Fit()
{
    ROOT::EnableImplicitMT();

    // Analysis
    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "2H", "2H")};
    // Ex
    auto hEx {df.Histo1D(E796Fit::Exdd, "Ex")};
    // Phase space 19O
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "2H", 0, 1, 0)};
    auto hPS {phase.Histo1D(E796Fit::Exdd, "Eex", "weight")};
    Fitters::TreatPS(hEx.GetPtr(), hPS.GetPtr(), 2);

    // Sigmas
    Interpolators::Sigmas sigmas;
    sigmas.Read(gSelector->GetSigmasFile("2H", "2H").Data());

    // Interface to fit
    Fitters::Interface inter;
    double sigma {0.3}; // common init sigma for all
    inter.AddState("g0", {400, 0, sigma}, "0+");
    inter.AddState("g1", {100, 1.6, sigma}, "2+0");
    inter.AddState("g2", {50, 4, sigma}, "2+0");
    inter.AddState("g3", {50, 5.5, sigma}, "0+1");
    inter.AddState("g4", {50, 6.5, sigma}, "2+");
    inter.AddState("g5", {50, 7.6, sigma}, "3- and 4+");
    inter.AddState("g6", {50, 8.6, sigma}, "4+0");
    inter.AddState("g7", {50, 9.6, sigma}, "0+2");
    inter.AddState("ps0", {1.5}, "ps0");
    inter.EndAddingStates();
    // Read from previous fit
    inter.ReadPreviousFit("./Outputs/fit_" + gSelector->GetFlag() + ".root");
    // Sigma from interpolator
    inter.EvalSigma(sigmas.GetGraph());
    inter.SetFixAll(2, true);
    // Save to be used later
    inter.Write("./Outputs/interface.root");

    // Model
    Fitters::Model model {inter.GetNGauss(), inter.GetNVoigt(), {*hPS}};

    // Fitting range
    double exmin {-5};
    double exmax {25};

    // Run!
    Fitters::RunFit(hEx.GetPtr(), exmin, exmax, model, inter.GetInitial(), inter.GetBounds(), inter.GetFixed(),
                    ("./Outputs/fit_" + gSelector->GetFlag() + ".root"), "20O(d,d) fit",
                    {{"g0", "g.s"}, {"g1", "1st ex"}, {"ps0", "1-n phase"}});

    gSelector->SendToWebsite("dd.root", gPad);
}

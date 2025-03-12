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

    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "1H", "1H")};
    // Ex
    auto hEx {df.Histo1D(E796Fit::Expp, "Ex")};
    // Phase space deuton breakup
    // ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "2H", 0, -1, 0)};
    ROOT::RDataFrame phase {"SimulationTTree", "../../Simulation/Macros/Breakup/Outputs/d_breakup_trans.root"};
    auto hPS {phase.Histo1D(E796Fit::Expp, "Eex", "weight")};
    Fitters::TreatPS(hEx.GetPtr(), hPS.GetPtr(), 0);
    Fitters::FitPS(hPS.GetPtr(), "pol8", false, true);

    // Sigmas
    Interpolators::Sigmas sigmas;
    sigmas.Read(gSelector->GetSigmasFile("1H", "1H").Data());

    // Init interface
    Fitters::Interface inter;
    inter.AddState("g0", {400, 0, 0.3}, "0+");
    inter.AddState("g1", {100, 1.67, 0.3}, "2+");
    inter.AddState("g2", {50, 4.1, 0.3}, "2+");
    inter.AddState("g3", {50, 5.6, 0.3}, "3-");
    inter.AddState("g4", {50, 7.8, 0.3}, "3- and 4+");
    inter.AddState("ps0", {0.455});
    inter.EndAddingStates();
    // Read previous fit
    inter.ReadPreviousFit("./Outputs/fit_juan_RPx.root");
    // Eval sigma from interpolator
    inter.EvalSigma(sigmas.GetGraph());
    // And fix it!
    inter.SetFix("g1", 2, true);
    inter.SetFix("g2", 2, true);
    inter.SetFix("g3", 2, true);
    inter.SetFix("g4", 2, true);
    // inter.SetFix("ps0", 0, true);
    inter.Print();
    inter.Write("./Outputs/interface.root");

    // Model
    Fitters::Model model {inter.GetNGauss(), inter.GetNVoigt(), {*hPS}, inter.GetCte()};

    // Fitting range
    double exmin {-10};
    double exmax {12};

    // Run!
    Fitters::RunFit(hEx.GetPtr(), exmin, exmax, model, inter.GetInitial(), inter.GetBounds(), inter.GetFixed(),
                    ("./Outputs/fit_" + gSelector->GetFlag() + ".root"), "20O(p,p) fit",
                    {{"g0", "g.s"}, {"g1", "1st ex"}, {"ps0", "20O(d,d) breakup"}}, true);

    // gPad->GetListOfPrimitives()->RemoveLast();
    // gSelector->SendToWebsite("pp.root", gPad, "cFit");
}

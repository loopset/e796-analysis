#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TROOT.h"
#include "TString.h"

#include "FitInterface.h"
#include "FitModel.h"
#include "FitRunner.h"
#include "FitUtils.h"
#include "Interpolators.h"

#include <stdexcept>
#include <string>
#include <utility>
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
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "2H", 0, -1, 0)};
    auto hPS {phase.Histo1D(E796Fit::Expp, "Eex", "weight")};
    Fitters::TreatPS(hEx.GetPtr(), hPS.GetPtr(), 2);

    // Sigmas
    Interpolators::Sigmas sigmas;
    sigmas.Read(gSelector->GetSigmasFile("1H", "1H").Data());

    // Init interface
    Fitters::Interface inter;
    inter.AddState("g0", {400, 0, 0.3}, "0+");
    inter.AddState("g1", {100, 1.7, 0.3}, "2+");
    inter.AddState("ps0", {1.5});
    inter.EndAddingStates();
    // Eval sigma from interpolator
    for(const auto& key : inter.GetKeys())
        inter.SetInitial(key, 2, sigmas.Eval(inter.GetGuess(key)));
    // And fix it!
    inter.SetFix("g1", 2, true);
    inter.Print();
    inter.Write("./Outputs/interface.root");

    // Model
    Fitters::Model model {inter.GetNGauss(), inter.GetNVoigt(), {*hPS}, inter.GetCte()};

    // Fitting range
    double exmin {-5};
    double exmax {10};

    // Run!
    Fitters::RunFit(hEx.GetPtr(), exmin, exmax, model, inter.GetInitial(), inter.GetBounds(), inter.GetFixed(),
                    ("./Outputs/fit_" + gSelector->GetFlag() + ".root"), "20O(p,p) fit",
                    {{"g0", "g.s"}, {"g1", "1st ex"}, {"ps0", "20O(d,d) breakup"}});
}

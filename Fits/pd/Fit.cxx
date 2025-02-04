#include "ROOT/RDataFrame.hxx"

#include "TROOT.h"
#include "TVirtualPad.h"

#include "FitInterface.h"
#include "FitModel.h"
#include "FitUtils.h"
#include "Interpolators.h"

#include <string>
#include <vector>

#include "/media/Data/E796v2/Fits/FitHist.h"
#include "/media/Data/E796v2/Selector/Selector.h"
void Fit()
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3, "20O", "1H", "2H")};
    auto hEx {df.Histo1D(E796Fit::Expd, "Ex")};
    // Phase space
    ROOT::RDataFrame phase {"SimulationTTree", gSelector->GetSimuFile("20O", "2H", "2H", 0, -2)};
    auto hPS {phase.Histo1D(E796Fit::Expd, "Eex")};
    Fitters::TreatPS(hEx.GetPtr(), hPS.GetPtr());

    // Init interface
    Fitters::Interface inter;
    double sigma {0.240};
    inter.AddState("g0", {2000, 0, sigma}, "5/2+");
    inter.AddState("g1", {250, 1.4, sigma}, "1/2+");
    inter.AddState("g2", {110, 3.1, sigma}, "5/2+");
    inter.AddState("ps0", {1});
    inter.EndAddingStates();
    inter.ReadPreviousFit("./Outputs/fit_" + gSelector->GetFlag() + ".root");
    // Set simulated sigmas
    inter.EvalSigma(Interpolators::Sigmas(gSelector->GetSigmasFile("1H", "2H").Data()).GetGraph());
    // Fix sigmas
    inter.SetFixAll(2, true);
    inter.Write("./Outputs/interface.root");

    // Fitting range
    double exmin {-10};
    double exmax {5};

    // Model
    Fitters::Model model {inter.GetNGauss(), inter.GetNVoigt(), {*hPS}, inter.GetCte()};

    // Run for all the cuts
    Fitters::RunFit(hEx.GetPtr(), exmin, exmax, model, inter.GetInitial(), inter.GetBounds(), inter.GetFixed(),
                    "./Outputs/fit_" + gSelector->GetFlag() + ".root", "20O(p,d)");

    gPad->GetListOfPrimitives()->RemoveLast();
    gSelector->SendToWebsite("pd.root", gPad, "cFit");
}

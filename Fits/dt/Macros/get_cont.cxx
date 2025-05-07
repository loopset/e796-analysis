#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include "FitInterface.h"
#include "FitUtils.h"
#include "Interpolators.h"

#include "../../../Selector/Selector.h"
#include "../../FitHist.h"
void get_cont()
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Sel_Tree", "../../../PostAnalysis/RootFiles/Pipe3/pd_as_dt_contamination.root"};
    auto hEx {df.Histo1D(E796Fit::Exdt, "Ex")};
    // (d,d) 1n PS as (d,t)
    ROOT::RDataFrame dps {"SimulationTTree", "../../../Simulation/Outputs/juan_RPx/tree_dd_1nps_as_dt.root"};
    auto hPS {dps.Histo1D(E796Fit::Exdt, "Eex", "weight")};
    Fitters::TreatPS(hEx.GetPtr(), hPS.GetPtr());

    Interpolators::Sigmas sigmas {gSelector->GetSigmasFile("2H", "3H").Data()};

    Fitters::Interface inter;
    double sigma {0.364}; // common guess for all states
    inter.AddState("g0", {400, 16.3, sigma}, "gs");
    inter.AddState("g1", {250, 18.1, sigma}, "1st");
    inter.AddState("g2", {110, 20.4, sigma}, "2nd");
    inter.AddState("ps0", {0.1});
    inter.EndAddingStates();
    // Wider mean margin
    inter.SetOffsetMeanBounds(0.5);
    // Eval correct sigma
    inter.EvalSigma(sigmas.GetGraph());
    inter.Write("./Outputs/inter.root");


    // Fitting range
    double exmin {-5};
    double exmax {25};
    // Model
    Fitters::Model model {inter.GetNGauss(), inter.GetNVoigt(), {*hPS}};
    // Run!
    Fitters::RunFit(hEx.GetPtr(), exmin, exmax, model, inter.GetInitial(), inter.GetBounds(), inter.GetFixed(),
                    ("./Outputs/fit.root"), "20O(d,t) contamination");

}

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TFitResult.h"
#include "TH1.h"
#include "TROOT.h"
#include "TRatioPlot.h"

#include "Fit/BinData.h"
#include "Fit/DataOptions.h"
#include "Fit/DataRange.h"
#include "Fit/Fitter.h"
#include "Math/WrappedMultiTF1.h"

#include "HFitInterface.h"

#include <iostream>

#include "/media/Data/PhysicsClasses/src/FitModel.cxx"
#include "/media/Data/PhysicsClasses/src/Fitter.cxx"

void test()
{
    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {
        "Final_Tree", "/media/Data/E796v2/PostAnalysis/RootFiles/Legacy/tree_beam_20O_target_2H_light_3H_front.root"};
    int nbins {100};
    double hmin {-5};
    double hmax {25};
    auto hEx {df.Histo1D(
        {"hEx", TString::Format(";E_{x} [MeV];Counts / %.0f keV", (hmax - hmin) / nbins * 1E3), nbins, hmin, hmax},
        "Ex")};
    // Init fit
    double xmin {hmin};
    double xmax {10};

    ROOT::Fit::DataOptions opts;
    opts.fIntegral = true;
    ROOT::Fit::DataRange range {xmin, xmax};
    ROOT::Fit::BinData data {opts, range};
    ROOT::Fit::FillData(data, hEx.GetPtr());

    // Model
    int ng {1};
    int nv {0};
    bool cte {false};
    auto* model = new Fitters::Model {ng, nv, {}, cte};

    // std::vector<double> initial {5000, 0, 1};
    // ROOT::Math::WrappedParamFunction wrap {&model, model.NDim(), model.NPar(), &initial.front()};
    auto [f1, wrap] {model->Wrap(xmin, xmax)};

    ROOT::Fit::Fitter fitter;
    fitter.SetFunction(wrap, false);
    fitter.Config().ParSettings(0).SetValue(100);
    fitter.Config().ParSettings(1).SetValue(0);
    fitter.Config().ParSettings(2).SetValue(0.37);
    fitter.Config().ParSettings(2).Fix();
    fitter.LeastSquareFit(data);
    // fitter.Result().Print(std::cout);

    auto* res {new TFitResult {fitter.Result()}};
    res->Print();

    auto* hclone {(TH1D*)hEx->Clone()};
    hclone->GetListOfFunctions()->Add(f1);
    //
    // // Ratio plot
    auto* c0 {new TCanvas {"c0", "test"}};
    auto* ratio {new TRatioPlot {hclone, "", res}};
    ratio->Draw();

    // for(int i = 0; i < model.NPar(); i++)
    // {
    //     std::cout << "i : " << i << " name : " << model.ParameterName(i) << '\n';
    //     auto [t, idx] {model.GetTypeIdx(model.ParameterName(i))};
    //     std::cout << "Type : " << t << " idx : " << idx << '\n';
    // }
}

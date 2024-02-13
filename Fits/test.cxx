#include "ROOT/RDataFrame.hxx"

#include "TH1.h"
#include "TROOT.h"

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
    int nbins {100};
    double hmin {-5};
    double hmax {25};
    auto* h {new TH1D {"h", "h", nbins, hmin, hmax}};
    h->FillRandom("gaus", 5000);
    // Init fit
    double xmin {hmin};
    double xmax {10};

    ROOT::Fit::DataOptions opts;
    opts.fIntegral = true;
    ROOT::Fit::DataRange range {xmin, xmax};
    ROOT::Fit::BinData data {opts, range};
    ROOT::Fit::FillData(data, h);

    // Model
    int ng {1};
    int nv {0};
    bool cte {false};
    Fitters::Model model {ng, nv, {}, cte};

    auto* gaus {new TF1("gaus", "gaus", hmin, hmax)};
    ROOT::Math::WrappedMultiTF1 fitfunc {*gaus, static_cast<unsigned int>(gaus->GetNdim())};

    ROOT::Fit::Fitter fitter;
    fitter.SetFunction(model, false);
    fitter.Config().ParSettings(0).SetValue(5000);
    fitter.Config().ParSettings(1).SetValue(0);
    fitter.Config().ParSettings(2).SetValue(2);
    auto res {fitter.LeastSquareFit(data)};

    fitter.Result().Print(std::cout);

    h->Draw();

    // for(int i = 0; i < model.NPar(); i++)
    // {
    //     std::cout << "i : " << i << " name : " << model.ParameterName(i) << '\n';
    //     auto [t, idx] {model.GetTypeIdx(model.ParameterName(i))};
    //     std::cout << "Type : " << t << " idx : " << idx << '\n';
    // }
}

#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TFitResult.h"
#include "TH1.h"

#include "Fit/BinData.h"
#include "Fit/DataOptions.h"
#include "Fit/DataRange.h"
#include "Fit/Fitter.h"

#include "HFitInterface.h"

#include <iostream>

#include "/media/Data/PhysicsClasses/src/FitFCN.cxx"
#include "/media/Data/PhysicsClasses/src/FitModel.cxx"
#include "/media/Data/PhysicsClasses/src/FitRunner.cxx"

void fcn()
{
    auto* h {new TH1D {"h", "h", 100, -5, 5}};
    h->FillRandom("gaus", 10000);

    // Data
    double xmin {-5};
    double xmax {5};
    ROOT::Fit::DataOptions opts;
    opts.fIntegral = false;
    ROOT::Fit::DataRange range {xmin, xmax};
    ROOT::Fit::BinData data {opts, range};
    ROOT::Fit::FillData(data, h);

    // Model
    int ng {1};
    int nv {0};
    bool cte {false};
    auto* model = new Fitters::Model {ng, nv, {}, cte};

    // Objective
    double pars[3] = {500, 0, 1};
    auto* obj = new Fitters::ObjFCN {data, *model};

    // Fitter
    ROOT::Fit::Fitter fitter;
    fitter.SetFCN(*obj, *model, pars, data.Size(), true);
    fitter.FitFCN();
    fitter.Result().Print(std::cout);

    // Fit result
    TFitResult res {fitter.Result()};

    auto* g {new TGraph};
    res.Scan(0, g);


    auto* c0 {new TCanvas {"c0"}};
    g->Draw("apl");


    // Check fit
    auto* f1 {new TF1 {"f1", [=](double* x, double* p) { return (*model)(x, p); }, xmin, xmax, (int)model->NPar()}};
    f1->SetParameters(res.GetParams());

    auto* c1 {new TCanvas {"c1"}};
    h->Draw();
    f1->SetLineColor(kRed);
    f1->Draw("same");
}

#ifndef FitTH2_h
#define FitTH2_h

#include "TF1.h"
#include "TH2.h"
#include "TList.h"
#include "TMath.h"
#include "TString.h"

#include "Fit/Fitter.h"
#include "Math/Functor.h"

#include <iostream>
#include <vector>
class FitTH2
{
    TH2* fHist {};
    int fNPar {};
    int fNPoints {};

public:
    FitTH2() = default;
    FitTH2(TH2* h, int npar) : fHist(h), fNPar(npar) {}
    double operator()(const double* p);
    TF1* Fit();
};

inline double FitTH2::operator()(const double* p)
{
    double chi {};
    int ndata {};
    auto nx {fHist->GetNbinsX()};
    auto ny {fHist->GetNbinsY()};
    for(int i = 1; i <= nx; i++)
    {
        for(int j = 1; j <= ny; j++)
        {
            auto c {fHist->GetBinContent(i, j)};
            if(!c)
                continue;
            auto x {fHist->GetXaxis()->GetBinCenter(i)};
            auto y {fHist->GetYaxis()->GetBinCenter(j)};
            double fx {};
            for(int i = 0; i < fNPar; i++)
                fx += p[i] * TMath::Power(x, i);
            auto num {TMath::Power(y - fx, 2)};
            auto denom {TMath::Power(1. / c, 2)};
            // INFO: weighting by the bin contents
            // rather that sqrt(counts) seems to improve the fit
            // and make it more robust against outliers
            chi += num / denom;
            ndata++;
        }
    }
    fNPoints = ndata;
    return chi;
}

inline TF1* FitTH2::Fit()
{
    ROOT::Math::Functor fcn {this, &FitTH2::operator(), static_cast<unsigned int>(fNPar)};
    ROOT::Fit::Fitter fitter;
    std::vector<double> init(fNPar);
    fitter.SetFCN(fcn, init.data());
    for(int i = 0; i < fNPar; i++)
        fitter.Config().ParSettings(i).SetName(TString::Format("p%d", i).Data());
    bool ok {fitter.FitFCN()};
    TF1* f {};
    if(ok)
    {
        auto res {fitter.Result()};
        res.Print(std::cout);
        f = new TF1 {"fitth2", TString::Format("pol%d", fNPar - 1), fHist->GetXaxis()->GetXmin(),
                     fHist->GetXaxis()->GetXmax()};
        f->SetFitResult(res);
        f->SetNDF(fNPoints - fNPar);
        fHist->GetListOfFunctions()->Add(f);
    }
    return f;
}
#endif

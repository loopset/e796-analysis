#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"
#include "TF1.h"
#include "TGraph.h"

#include "/media/Data/PhysicsClasses/src/Fitter.cxx"
#include "/media/Data/PhysicsClasses/src/PublicationColors.cxx"

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

void Fit_dt()
{
    ROOT::EnableImplicitMT();
    
    gROOT->SetStyle("Plain");
    ROOT::RDataFrame df {"yield_tree", "/media/miguel/FICA_4/JuanCode/20O_dt_19O_21_Feb_23_v1.root"};
    //df.Describe().Print();std::cout<<'\n';
    //Apply mass cuts from Juan
    auto applyMassCuts = [&](double AmassH, double thetaCM, double Ex)
    {
        //fits to mass gaussians
        //A = 2
        double A2mass {2.05609 - 0.0328848 * Ex + 0.00288087 * Ex *Ex};
        double S2mass {0.251386 + 0.0171722 * Ex - 0.00191928 * Ex * Ex};
        //A = 3
        double A3mass {3.16142 - 0.034846 * Ex + 0.000415012 * Ex * Ex};
        double S3mass {0.306103 - 0.00590731 * Ex - 0.000331309 * Ex * Ex};

        int kA2 {1}; int kA3 {2};
        //Mass cut
        bool A2cut {(A2mass - kA2 * S2mass < AmassH) && (AmassH < A2mass + kA2 * S2mass)};
        bool A3cut {(A3mass - kA3 * S3mass < AmassH) && (AmassH < A3mass + kA3 *S3mass)};
        //Cut in thetaCM (only centered detectors)
        bool thetaCMcut {4. <= thetaCM && thetaCM <= 13};
        //Ex less than 10 MeV
        bool Excut {Ex < 10};

        return (A3cut && thetaCMcut && Excut);
    };
    auto gated {df.Filter(applyMassCuts, {"Amass_Hlike", "ThetaCM", "Ex"})};
    std::cout<<"Counts after cuts = "<<gated.Count().GetValue()<<'\n';
    
    int nbins {100};
    double hmin {-5}; double hmax {25};
    auto hEx {gated.Histo1D({"hEx", TString::Format(";E_{x} [MeV];Counts / %.0f keV", (hmax - hmin) / nbins * 1E3),
                nbins, hmin, hmax},
            "Ex")};
    //Read PS
    ROOT::RDataFrame phase {"simulated_tree", "/media/miguel/FICA_4/FitJuan/20O_and_2H_to_3H_NumN_1_NumP_0_Ex0_Date_2022_11_29_Time_16_35.root"};
    auto hPS {phase.Histo1D({"hPS", "PS 1n;E_{x} [MeV]", nbins, hmin, hmax}, "Ex_cal")};
    //Format phase space
    hPS->Smooth(20);
    //Scale it
    auto intEx {hEx->Integral()};
    auto intPS {hPS->Integral()};
    double factor {0.15};
    hPS->Scale(factor * intEx / intPS);
    
    //Init fit
    double xmin {hmin};
    double xmax {10};
    Fitters::SpectrumData data {xmin, xmax, hEx.GetPtr()};
    data.AddPhaseSpace(hPS.GetPtr());
    int ngauss {7};
    Fitters::SpectrumFunction func {ngauss, 0, &data};
    Fitters::SpectrumFitter fitter {&func};
    //Set init parameters
    double sigma {0.374};
    Fitters::SpectrumFitter::InitPars initPars {
        {"g0", {400, 0, sigma}},
        {"g1", {10, 1.5, sigma}},
        {"g2", {110, 3.2, sigma}},
        {"g3", {60, 4.4, sigma}},
        {"g4", {56, 5.2, sigma}},
        {"g5", {60, 6.9, sigma}},
        {"g6", {65, 9., sigma}},
        {"ps0", {0.1}},
    };
    //Set bounds and fix parameters
    double minmean {0.3}; double maxmean {0.3};
    Fitters::SpectrumFitter::InitBounds initBounds {};
    Fitters::SpectrumFitter::FixedPars fixedPars {};
    for(const auto& [key, init] : initPars)
    {
        //determine number of parameters
        auto strkey {TString(key)};
        int npars {};
        if(strkey.Contains("g"))
            npars = 3;//gaussian
        else if(strkey.Contains("v"))
            npars = 4;//voigt
        else if(strkey.Contains("ps"))
            npars = 1;
        else
            throw std::runtime_error("Wrong key received in initPars");
        for(int par = 0; par < npars; par++)
        {
            std::pair<double, double> pair {};//for bounds
            bool boo {};//for fix parameters
            if(par == 0)//Amplitude
            {
                pair = {0, 1000};
                boo = false;
            }
            else if(par == 1)//Mean
            {
                pair = {init[par] - minmean, init[par] + maxmean};
                boo = false;
            }
            else if(par == 2)//Sigma
            {
                pair = {-11, -11};
                boo = true;//fix it
            }
            else
                throw std::runtime_error("No automatic config for this parameter index (only gaussians so far)!");
            //Fill
            initBounds[key].push_back(pair);
            fixedPars[key].push_back(boo);
        }
    }
    fitter.SetInitPars(initPars);
    fitter.SetInitBounds(initBounds);
    fitter.SetFixedPars(fixedPars);
    //Fit!!
    fitter.Fit();

    //Draw
    Fitters::SpectrumPlotter plotter {&data, &func, fitter.GetFitResult()};
    auto* gfit {plotter.GetGlobalFitGraph()};
    auto hfits {plotter.GetIndividualHists()};
    auto* hps0fit {plotter.GetIndividualPS(0)};
    
    //plotting
    PlotUtils::PublicationColors pubcol;
    auto* cex {new TCanvas("cex")};
    cex->cd();
    //Settings for juan's presentation
    hEx->SetStats(false);
    hEx->SetLineWidth(2);
    hEx->DrawClone("hist");
    //1->Draw global fit
    gfit->SetLineColor(pubcol[5]); gfit->SetLineWidth(2);
    gfit->Draw("same");
    int idx {};
    std::vector<int> colors {pubcol[4], pubcol[3], pubcol[2], pubcol[0], pubcol[1],
        pubcol[4], pubcol[3]};
    std::vector<int> fs {3244, 3295, 3245, 3254, 3205,
        3244, 3295};
    for(auto& [_, h] : hfits)
    {
        h->SetLineWidth(2);
        h->SetLineColor(colors.at(idx));
        h->SetFillStyle(fs.at(idx));
        h->SetFillColor(colors.at(idx));
        // int color {idx + 6};
        // if(color == 10)
        //     color = 46;
        // g->SetLineColor(color);
        h->Draw("hist same");
        idx++;
    }
    hps0fit->SetLineColor(kMagenta - 3);
    hps0fit->SetFillColor(kMagenta - 3);
    hps0fit->SetFillStyle(3325);
    hps0fit->Draw("hist same");
}

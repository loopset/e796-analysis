#include "ActKinematics.h"
#include "ActParticle.h"
#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TVirtualPad.h"
#include "TLine.h"
#include "ActColors.h"

#include <iostream>
#include <string>
#include <vector>
void Plotter(const std::vector<double>& Exs,
             const std::string& beam, const std::string& target,
             const std::string& light, const std::string& heavy,
             double T1,
             int neutronPS, int protonPS)
{
    ROOT::EnableImplicitMT();

    //Set if we should fit or not
    bool isPS {(neutronPS != 0 || protonPS != 0)};
    if(!isPS)
        std::cout<<BOLDCYAN<<"Plotting usual simulation"<<RESET<<'\n';
    else
        std::cout<<BOLDCYAN<<"Plotting a phase space"<<RESET<<'\n';
    //Binning settings form TH1,2DModel
    int nBinsEx {(isPS) ? 1000 : 500};//for PS we need more binning to appreciate the rise after Sn
    int nBinsEVertex {120};
    int nBinsTheta {400};
    
    //Save histograms
    std::vector<TH1D*> hsEx {};
    std::vector<TH2D*> hsKin {};
    //Iterate
    int idx {1};
    for(const auto& Ex : Exs)
    {
        //Get DF
        ROOT::RDataFrame df("SimulationTTree", TString::Format("/media/Data/E796v2/Simulation/Outputs/transfer_TRIUMF_Eex_%.3f_nPS_%d_pPS_%d.root", Ex, neutronPS, protonPS));
           
        //Book histograms
        auto hEx {df.Histo1D({TString::Format("hEx%d", idx), TString::Format("Ex = %.2f MeV with nPS = %d and pPS = %d;E_{ex} [MeV]", Ex, neutronPS, protonPS),
                                      nBinsEx, -10., 60.},
                "Eex", "weight")};
        auto hKin {df.Histo2D({TString::Format("hKin%d", idx),
                                       TString::Format("Ex = %.2f MeV with nPS = %d and pPS = %d;#theta_{Lab} [#circ];E_{Vertex} [MeV]", Ex, neutronPS, protonPS),
                                       nBinsTheta, 0., 180., nBinsEVertex, 0., 80.},
                "theta3Lab", "EVertex", "weight")};
                
        //clone in order to save
        hsEx.push_back((TH1D*)hEx->Clone());
        hsKin.push_back((TH2D*)hKin->Clone());
        idx++;
    }
    //Fit to gaussians!
    auto* gsigma {new TGraphErrors()};
    double range {4.5};//MeV autour de la moyenne
    for(int i = 0; i < hsEx.size(); i++)
    {
        if(isPS)
            continue;
        hsEx[i]->Fit("gaus", "0Q", "", Exs[i] - range, Exs[i] + range);
        auto* f {hsEx[i]->GetFunction("gaus")};
        gsigma->SetPoint(gsigma->GetN(), Exs[i], f->GetParameter("Sigma"));
        gsigma->SetPointError(gsigma->GetN() - 1, 0, f->GetParError(2));
    }

    //plot!
    auto* cfits {new TCanvas("cfits", "Canvas with fits")};
    cfits->DivideSquare(hsEx.size());
    for(int i = 0; i < hsEx.size(); i++)
    {
        cfits->cd(i + 1);//pads are named in range [1, max], not [0, max-1]
        hsEx[i]->Draw("hist");
        for(auto* o : *(hsEx[i]->GetListOfFunctions()))
            if(o)
                o->Draw("same");//draw fitted function stored in list of funcs of histogram after fit
        gPad->Update();
        if(isPS)
            continue;
        //draw a line centered at Exs[i] to compare gaussian to where its center should be
        auto* line {new TLine(Exs[i], gPad->GetUymin(), Exs[i], gPad->GetUymax())};
        line->SetLineWidth(2); line->SetLineColor(kMagenta);
        line->Draw("same");
    }

    //Get total kinematic energy of beam (T1 * mass of beam)
    ActPhysics::Particle p1 {beam};
    double T1Total {T1 * p1.GetAMU()};
    auto* ckin {new TCanvas("ckin", "Kinematic canvas")};
    ckin->DivideSquare(hsKin.size());
    for(int i = 0; i < hsKin.size(); i++)
    {
        ckin->cd(i + 1);
        hsKin[i]->Draw("colz");
        //Draw kinematic lines
        ActPhysics::Kinematics kin(beam, target, light, heavy, T1Total, Exs[i]);
        auto* gtheo {kin.GetKinematicLine3()};
        gtheo->Draw("same");
    }

    if(isPS)
        return;
    auto* csigma {new TCanvas("csigma", "Sigmas from fits")};
    gsigma->SetTitle(";E_{x} [MeV];#sigma in E_{x} [MeV]");
    gsigma->SetMarkerStyle(24);
    gsigma->SetLineColor(kViolet);
    gsigma->SetLineWidth(2);
    gsigma->Draw("apl0");
}

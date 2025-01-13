#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"

#include <string>
#include <vector>

#include "../../../PostAnalysis/HistConfig.h"
#include "../../../Selector/Selector.h"

void comp()
{
    std::string target {"1H"};
    std::string light {"1H"};
    double Ex {0};

    // Get EXPERIMENTAL
    ROOT::RDataFrame exp {"Sel_Tree", gSelector->GetAnaFile(3, "20O", target, light)};
    // Get SIMULATION
    ROOT::RDataFrame simu {"SimulationTTree", gSelector->GetSimuFile("20O", target, light, Ex)};
    // Book histograms
    std::vector<TH2D*> hsKinLab, hsKinCM;
    std::vector<TH1D*> hsRPx;
    std::vector<ROOT::RDF::RNode> dfs {exp, simu};
    int idx {};
    for(auto& df : dfs)
    {
        auto isSimu {idx > 0};
        auto title {isSimu ? "simu" : "ana"};
        auto hKinLab {df.Histo2D(HistConfig::ChangeTitle(HistConfig::KinEl, title),
                                 isSimu ? "theta3Lab" : "fThetaLight", "EVertex")};
        auto hKinCM {
            df.Histo2D(HistConfig::ChangeTitle(HistConfig::KinCM, title), isSimu ? "theta3CM" : "ThetaCM", "EVertex")};
        auto hRPx {df.Histo1D(HistConfig::ChangeTitle(HistConfig::RPx, title), isSimu ? "RPx" : "fRP.fCoordinates.fX")};

        // add to vectors
        hsKinLab.push_back((TH2D*)hKinLab->Clone());
        hsKinCM.push_back((TH2D*)hKinCM->Clone());
        hsRPx.push_back((TH1D*)hRPx->Clone());
        idx++;
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "Debug efficiency canvas"}};
    c0->DivideSquare(6);
    for(int i = 0; i < hsKinLab.size(); i++)
    {
        c0->cd(i + 1);
        hsKinLab[i]->Draw("colz");
    }
    for(int i = 0; i < hsKinCM.size(); i++)
    {
        c0->cd(i + 3);
        hsKinCM[i]->Draw("colz");
    }
    for(int i = 0; i < hsRPx.size(); i++)
    {
        c0->cd(5);
        if(i == 0)
        {
            hsRPx[i]->SetLineColor(8);
            hsRPx[i]->DrawNormalized();
        }
        hsRPx[i]->DrawNormalized("same");
    }
}

#include "ActKinematics.h"

#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TMultiGraph.h"
#include "TString.h"
#include "TVirtualPad.h"
#include "TLegend.h"

#include <memory>
#include <string>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
#include "../../Selector/Selector.h"
void kin_elastic()
{
    std::vector<std::vector<std::string>> files;
    files.push_back(gSelector->GetSimuFiles("20O", "1H", "1H"));
    files.push_back(gSelector->GetSimuFiles("20O", "2H", "2H"));
    std::vector<std::string> labels {"pp", "dd"};
    std::vector<ActPhysics::Kinematics> kins {ActPhysics::Kinematics {"20O(p,p)@700"},
                                              ActPhysics::Kinematics {"20O(d,d)@700"}};
    std::vector<std::vector<double>> exs {{0, 1.67, 4.1, 5.6, 7.8}, {0, 1.6, 4.0, 5.5, 6.5, 7.6, 8.6, 9.6, 10.4, 11.7}};

    // Create histograms
    ROOT::RDF::TH2DModel mKin {"hKin", "Elastic kin;#theta_{Lab} [#circ];E_{Lab} [MeV]", 600, 0, 90, 600, 0, 12};
    std::vector<std::shared_ptr<TH2D>> hs;
    std::vector<TMultiGraph*> mgs;
    for(int i = 0; i < labels.size(); i++)
    {
        hs.push_back(mKin.GetHistogram());
        hs.back()->SetTitle(labels[i].c_str());
        mgs.push_back(new TMultiGraph);
        // Parse each file
        int j {};
        for(const auto& file : files[i])
        {
            ROOT::RDataFrame df {"SimulationTTree", file};
            auto hKin {df.Histo2D(mKin, "theta3Lab", "EVertex")};
            hs.back()->Add(hKin.GetPtr());
            // Kinematic line
            kins[i].SetEx(exs[i][j]);
            auto* g {kins[i].GetKinematicLine3()};
            g->SetTitle(TString::Format("%.2f MeV", exs[i][j]));
            mgs[i]->Add(g, "l");
            j++;
        }
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "Simulated kinematics"}};
    c0->DivideSquare(2);
    for(int i = 0; i < hs.size(); i++)
    {
        c0->cd(i + 1);
        hs[i]->SetStats(false);
        hs[i]->DrawClone("colz");
        mgs[i]->Draw("l plc pmc");
        auto* leg {gPad->BuildLegend()};
        leg->GetListOfPrimitives()->RemoveFirst();

    }
}

#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TVirtualPad.h"

#include <string>
#include <vector>

void plot()
{
    ROOT::EnableImplicitMT();

    std::vector<std::string> labels {"(d,p)", "(d,d)", "(d,t)"};
    std::vector<std::string> files {"./Outputs/tree_11Li_2H_1H_Ex_0.00.root", "./Outputs/tree_11Li_2H_2H_Ex_0.00.root",
                                    "./Outputs/tree_11Li_2H_3H_Ex_0.00.root"};

    ROOT::RDF::TH2DModel model {"hdE01", "E0-E1;E_{0} [MeV];E_{1} [MeV]", 300, 0, 40, 300, 0, 40};
    auto hSum {model.GetHistogram()};
    std::vector<TGraph*> gs;
    auto* mg {new TMultiGraph};
    mg->SetTitle("E0-E1;E_{0} [MeV];E_{1} [MeV]");
    for(int i = 0; i < labels.size(); i++)
    {
        auto& file {files[i]};
        ROOT::RDataFrame df {"SimulationTree", file};
        auto hdE01 {df.Histo2D(model, "dE0", "dE1")};
        auto gdE01 {df.Graph("dE0", "dE1")};
        hdE01->SetTitle(labels[i].c_str());
        gdE01->SetTitle(labels[i].c_str());

        hSum->Add(hdE01.GetPtr());
        gs.push_back((TGraph*)gdE01->Clone());
        mg->Add(gs.back());
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "Sum canvas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hSum->DrawClone("colz");
    c0->cd(2);
    mg->Draw("ap pmc plc pfc");
    gPad->BuildLegend();
}

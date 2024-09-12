#include "TFile.h"
#include "TGraphErrors.h"
#include "TLegend.h"
#include "TMultiGraph.h"

#include <vector>
void mg()
{
    // Lemos os gráficos coas contas
    auto* fin {new TFile {"./graphs.root"}};
    auto* g0 {fin->Get<TGraphErrors>("g0")};
    auto* g1 {fin->Get<TGraphErrors>("g1")};
    auto* g2 {fin->Get<TGraphErrors>("g2")};

    // Creamos o multigraph
    auto* mg {new TMultiGraph};
    mg->SetTitle("Counts per peak;#theta_{CM} [deg];Counts");

    // Estilos
    std::vector<int> colors {1, 2, 3};
    std::vector<int> ms {24, 25, 27};

    // Lenda
    auto* leg {new TLegend {0.3, 0.3}};
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->SetHeader("E_{x} [MeV]", "C");
    // Labels
    std::vector<std::string> labels {"g.s", "0.130", "0.435"};

    // Set e add to multigraphs
    int idx {};
    for(auto* g : {g0, g1, g2})
    {
        g->SetLineColor(colors[idx]);
        g->SetLineWidth(2);
        g->SetMarkerStyle(ms[idx]);
        g->SetMarkerColor(colors[idx]);

        // Add
        mg->Add(g, "lpe"); // l = line, p = point (marker), e = show error bar also
        // Legend
        leg->AddEntry(g, labels[idx].c_str(), "lpe");

        idx++;
    }

    // Draw
    // O multigraph computa automaticamente o rango adecuado para que todos os graphs se vexan
    mg->Draw("apl");
    // se engades as opcións plc pmc -> vai coller automaticamente as cores da paleta de cores actual
    // moi útil para o futuro
    // mg->Draw("apl plc pmc");
    leg->Draw();
}

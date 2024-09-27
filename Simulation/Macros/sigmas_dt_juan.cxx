#include "Rtypes.h"

#include "TGraph.h"

#include <vector>
void sigmas_dt_juan()
{
    std::vector<double> x {0, 1.47, 3.24, 4.4, 5.2, 6.9, 10, 12.8, 14.9};
    std::vector<double> y {0.388, 0.395, 0.390, 0.391, 0.381, 0.379, 0.377, 0.372, 0.356};

    auto* g {new TGraph {(int)x.size(), x.data(), y.data()}};
    g->SetTitle("Juan's 20O(d,t) sigma;E_{x} [MeV];#sigma [MeV]");
    g->SetLineWidth(2);
    g->SetLineColor(kMagenta);
    g->SetMarkerStyle(24);

    g->Draw("apl");
}

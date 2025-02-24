#include "TStyle.h"
#include "ModelPlotter.h"

#include <string>
#include <vector>
void z8_systematics()
{
    gStyle->SetTextFont(132);
    gStyle->SetTextSize(0.035);
    std::vector<std::string> jpis {"5/2+", "1/2-"};
    std::vector<int> colors {8, 9};
    // 17O(p,d)
    PlotUtils::ModelToPlot o17pd {"^{18}O(p,d)"};
    o17pd.SetEx({0, 3.050});
    o17pd.SetSF({"1.31", "0.88"});
    // 17O(p,d)
    PlotUtils::ModelToPlot o17dt {"^{18}O(d,t)"};
    o17dt.SetEx({0, 3.055});
    o17dt.SetSF({"1.48(27)", "1.08"});
    // 16O(d,p)
    PlotUtils::ModelToPlot o16dp {"^{16}O(d,p)"};
    o16dp.SetEx({0, 3.055});
    o16dp.SetSF({"0.84(04)", "0.032"});

    // 20O(d,t)
    PlotUtils::ModelToPlot o20dt {"Our (d,t)"};
    o20dt.SetEx({0, 3.141});
    o20dt.SetSF({"2.820(62)", "0.777(36)"});

    // 20O(d,p)
    PlotUtils::ModelToPlot o20dp {"^{20}O(d,p)"};
    o20dp.SetEx({0});
    o20dp.SetSF({"0.34(08)"});

    // Attach common info
    std::vector<PlotUtils::ModelToPlot*> models {&o16dp, &o17pd, &o17dt, &o20dt, &o20dp};
    for(auto m : models)
    {
        m->SetJp(jpis);
        m->SetColors(colors);
    }

    // Init plotter
    PlotUtils::ModelPlotter plotter {0, 10, static_cast<int>(models.size())};
    plotter.SetYaxisLabel("E_{x} [MeV]");
    for(const auto& m : models)
        plotter.AddModel(*m);

    plotter.Draw();
}

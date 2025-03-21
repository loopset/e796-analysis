#include "ActParticle.h"

#include "TStyle.h"

#include "ModelPlotter.h"

#include <string>
#include <vector>
void z8_systematics()
{
    gStyle->SetTextFont(132);
    gStyle->SetTitleX(0.55);
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
    // // 16O(d,p)
    // PlotUtils::ModelToPlot o16dp {"^{16}O(d,p)"};
    // o16dp.SetEx({0, 3.055});
    // o16dp.SetSF({"0.84(04)", "0.032"});

    // 20O(d,t)
    PlotUtils::ModelToPlot o20dt {"^{20}O(d,t)"};
    o20dt.SetEx({0, 3.141});
    o20dt.SetSF({"2.820(62)", "0.777(36)"});

    // // 20O(d,p)
    // PlotUtils::ModelToPlot o20dp {"^{20}O(d,p)"};
    // o20dp.SetEx({0});
    // o20dp.SetSF({"0.34(08)"});

    // Attach common info
    // Relative to Sn
    ActPhysics::Particle o17 {"17O"};
    ActPhysics::Particle o19 {"19O"};
    std::vector<ActPhysics::Particle*> ps {&o17, &o17, &o19};
    std::vector<PlotUtils::ModelToPlot*> models {&o17pd, &o17dt, &o20dt};
    int i {};
    for(auto m : models)
    {
        for(auto& ex : m->GetExs())
            ex = -ps[i]->GetSn() + ex;
        m->SetJp(jpis);
        m->SetColors(colors);
        i++;
    }

    // Init plotter
    PlotUtils::ModelPlotter plotter {-8, 2, static_cast<int>(models.size()), "Z = 8 systematics"};
    plotter.SetYaxisLabel("S_{n}^{eff} [MeV]");
    for(const auto& m : models)
        plotter.AddModel(*m);

    plotter.Draw();
    // Draw line Ex = 0
    auto* l {new TLine {gPad->GetUxmin(), 0, gPad->GetUxmax(), 0}};
    l->SetLineWidth(2);
    l->SetLineStyle(2);
    l->Draw();
}

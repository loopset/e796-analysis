#include "ActParticle.h"

#include "TLine.h"
#include "TStyle.h"
#include "TVirtualPad.h"

#include "ModelPlotter.h"

#include <string>
#include <vector>
void n11_systematics()
{
    gStyle->SetTextFont(132);
    gStyle->SetTextSize(0.035);
    gStyle->SetTitleX(0.55);
    std::vector<std::string> jpis {"5/2+", "1/2-"};
    std::vector<int> colors {8, 9};
    // 22Ne(d,t)
    PlotUtils::ModelToPlot ne22dt {"^{22}Ne(d,t)"};
    ne22dt.SetEx({0.351, 2.789});
    ne22dt.SetSF({"2.16", "0.78"});
    // 22Ne(p,d)
    PlotUtils::ModelToPlot ne22pd {"^{22}Ne(p,d)"};
    ne22pd.SetEx({0.350, 2.790});
    ne22pd.SetSF({"2.5", "0.7"});

    // 20O(d,t)
    PlotUtils::ModelToPlot o20dt {"^{20}O(d,t)"};
    o20dt.SetEx({0, 3.141});
    o20dt.SetSF({"2.820(62)", "0.777(36)"});

    // Attach common info
    // Relative to Sn
    ActPhysics::Particle ne21 {"21Ne"};
    ActPhysics::Particle o19 {"19O"};
    std::vector<ActPhysics::Particle*> ps {&o19, &ne21, &ne21};
    std::vector<PlotUtils::ModelToPlot*> models {&o20dt, &ne22pd, &ne22dt};
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
    PlotUtils::ModelPlotter plotter {-8, 2, static_cast<int>(models.size()), "N = 11 systematics"};
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

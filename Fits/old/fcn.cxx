#include "TGraph.h"
#include "TH1D.h"

#include "FitData.h"
#include "FitModel.h"
#include "FitRunner.h"

#include <vector>

void fcn()
{
    auto* h {new TH1D {"h", "h", 100, -5, 5}};
    h->FillRandom("gaus", 10000);

    // Data
    double xmin {-5};
    double xmax {5};
    Fitters::Data data {*h, xmin, xmax};

    // Model
    int ng {1};
    int nv {0};
    bool cte {false};
    auto* model = new Fitters::Model {ng, nv, {}, cte};

    // Runner
    Fitters::Runner runner {data, *model};
    runner.GetFitter().Config().ParSettings(0).SetValue(100);
    runner.GetFitter().Config().ParSettings(1).SetValue(0);
    runner.GetFitter().Config().ParSettings(2).SetValue(1);
    runner.Fit();

    // test
    delete model;

    auto res {runner.GetFitResult()};
    auto* g {new TGraph};
    res.Scan(2, g);

    g->Draw("apl");
}

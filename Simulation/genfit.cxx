
#include "TFile.h"
#include "TF1.h"

#include "FitModel.h"
#include "FitRunner.h"
#include "FitUtils.h"

void genfit()
{
    auto* h {new TH1D {"h", "h", 400, -10, 10}};
    // Fill random
    auto* f {new TF1 {"f", "4 * TMath::Voigt(x, 0.5, 0.1)", -10, 10}};
    f->Draw();
    h->FillRandom("f", 100000);

    // Declare fitting range
    double exmin {-5};
    double exmax {5};

    // Model
    int ngauss {0};
    int nvoigt {1};
    bool cte {false};
    Fitters::Model model {ngauss, nvoigt, {}, cte};

    // Set initial parameters
    Fitters::Runner::Init initPars {
        {"v0", {10, 0., 0.09, 0.1}},
        // {"v1", {200, 0.13, 0.09, 0.1}},
        // {"v2", {120, 0.435, 0.09, 0.1}},
    };
    Fitters::Runner::Bounds initBounds {
        {"v0", {{0, 100000}, {-11, -11}, {0.01, 1}, {0.05, 0.2}}},
        // {"v1", {{0, 1000}, {0.08, 0.2}, {0.01, 1}, {0.05, 0.2}}},
        // {"v2", {{0, 1000}, {0.3, 0.5}, {0.01, 1}, {0.01, 0.2}}},
    };
    Fitters::Runner::Fixed fixedPars {
        {"v0", {false, false, false, false}},
        // {"v1", {false, false, false, false}},
        // {"v2", {false, false, false, false}},
    };
    Fitters::Runner::Step steps {
        {"v0", {1, 0.01, 0.01, 0.001}},
    };

    // Run fit
    Fitters::RunFit(h, exmin, exmax, model, initPars, initBounds, fixedPars, ("./Outputs/fit.root"), h->GetTitle(),
                    {{"g0", "g.s"}, {"g1", "E_{x} = 0.13 MeV"}, {"g2", "E_{x} = 0.435 MeV"}}, steps);
}

#include "ActSRIM.h"

#include "TFile.h"
#include "TH1.h"

#include "CalibrationRunner.h"
#include "CalibrationSource.h"

void CorrectSource(Calibration::Source* source, ActPhysics::SRIM* srim, const std::string& table, double thickness,
                   double angle = 0)
{
    auto& energies {source->GetRefToEnergies()};
    auto& limits {source->GetRefToLimits()};
    auto labels {source->GetLabels()};
    // Peak energies
    for(auto& source : energies)
        for(auto& energy : source)
            energy = srim->Slow(table, energy, thickness, angle);
    // Peak boundaries
    for(auto& [key, vals] : limits)
    {
        vals.first = srim->Slow(table, vals.first, thickness, angle);
        vals.second = srim->Slow(table, vals.second, thickness, angle);
    }
}

void strip_must2()
{
    auto* fin {new TFile {"/media/Data/E748/Calibrate/DSSD_E/Inputs/XE.root"}};
    // pick strip and tel
    auto* h {fin->Get<TH1D>("hT1XE25")};

    // Source of ganil
    Calibration::Source source {};
    // source.Print();

    // Correct by energy losses in Al dead layer
    ActPhysics::SRIM srim {"al", "../SRIMData/raw/4He_silicon.txt"};
    CorrectSource(&source, &srim, "al", 0.5e-3); // 0.5 um to mm
    // source.Print();

    // Rebin
    auto* hrebin {(TH1D*)h->Clone()};

    // Runner
    Calibration::Runner run {&source, h, hrebin, false};
    run.SetGaussPreWidth(20);
    run.SetRange(8800, 9000);
    run.DisableXErrors();
    run.DoIt();
    auto* c {new TCanvas};
    run.Draw(c);
    run.PrintRes();


    // Save to disk
    auto fout {std::make_unique<TFile>("/media/Data/E748/Calibrate/DSSD_E/Outputs/single_strinp_cal.root", "recreate")};
    run.GetHistFinal()->Write();
    for(auto& [key, funcs] : run.GetFinalSat())
        for(auto& func : funcs)
            func->Write();
}

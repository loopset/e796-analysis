#include "ActPIDCorrector.h"

#include "TFile.h"

#include <stdexcept>

void ChangeName()
{

    auto f {std::make_unique<TFile>("../../Calibrations/Actar/pid_corr_tritons_f0.root")};
    auto pid {std::shared_ptr<ActPhysics::PIDCorrection>(f->Get<ActPhysics::PIDCorrection>("PIDCorrection"))};
    if(!pid)
        throw std::runtime_error("Cannot read PIDCorrector");
    pid->SetName("f0");
    pid->Print();
    pid->Write("../../Calibrations/Actar/pid_corr_tritons_f0_ok.root");
}


#include "ActGenCorrection.h"

#include "TFile.h"

#include <memory>
#include <string>
#include <vector>

std::pair<TF1*, TF1*> Read(const std::string& file)
{
    auto f {std::make_unique<TFile>(file.c_str())};
    auto* f1 {f->Get<TF1>("func1")};
    auto* f2 {f->Get<TF1>("func2")};
    return {f1, f2};
}

void Write()
{
    // Adapts corrections for ActRoot
    std::vector<std::string> names {"f0", "l0"};
    std::vector<std::string> in {"./Outputs/angle_corr_front_v2.root", "./Outputs/angle_corr_side_v2.root"};
    std::vector<std::string> out {"./Outputs/angle_front.root", "./Outputs/angle_side.root"};

    for(int i = 0; i < names.size(); i++)
    {
        auto& file {in[i]};
        ActPhysics::GenCorrection corr {names[i]};
        auto [f1, f2] {Read(file)};
        corr.Add("f1", f1);
        corr.Add("f2", f2);
        corr.Print(true);
        corr.Write(out[i]);
    }
}

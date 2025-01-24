#include "ActSilMatrix.h"

#include "TCanvas.h"
#include "TMarker.h"

#include <utility>
#include <vector>

#include "../../../PostAnalysis/Utils.cxx"
void silMatrices()
{
    // Original matrix
    auto* sm {E796Utils::GetAntiVetoMatrix()};
    double silCentre {sm->GetMeanZ({3, 4})};
    double beamOffset {9.01};
    double xRef {181}; // pad units
    std::pair<double, double> yzRef {126, silCentre + beamOffset};

    // For different distances
    std::vector<ActPhysics::SilMatrix*> sms;
    std::vector<TMarker*> ms;
    double padSize {2}; // mm
    double pad {256};   // mm
    for(double d = 90; d <= 190; d += 15)
    {
        auto dist {pad + d};
        dist /= padSize;
        auto* clone {sm->Clone()};
        clone->MoveXYTo(xRef, yzRef, dist);
        clone->SetSyle(false);
        sms.push_back(clone);
        // Scale also reference point
        auto scale {dist / xRef};
        auto* m {new TMarker {yzRef.first * 1, clone->GetMeanZ({3,4}) + beamOffset, 24}};
        ms.push_back(m);
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "sil matrices canvas"}};
    sm->Draw(false);
    for(auto& m : sms)
        m->Draw();
    for(auto& m : ms)
        m->Draw();
}

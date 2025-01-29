#include "ActKinematics.h"

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TMath.h"

#include "../../PostAnalysis/HistConfig.h"
#include "../../Selector/Selector.h"
void contamination()
{
    // Get same cuts as for dt
    gSelector->SetTarget("2H");
    gSelector->SetLight("3H");

    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Sel_Tree", gSelector->GetAnaFile(3)};
    auto minE {df.Min("EVertex")};
    auto maxE {df.Max("EVertex")};
    auto minTheta {df.Min("fThetaLight")};
    auto maxTheta {df.Max("fThetaLight")};

    // Allegedly peak @ 16 MeV is contamination from (p,d) gs
    ActPhysics::Kinematics pd {"20O(p,d)@700"};
    auto gpd {pd.GetKinematicLine3()};

    // Reconstruct it as dt
    ActPhysics::Kinematics dt {"20O(d,t)@700"};
    auto hEx {HistConfig::Ex.GetHistogram()};
    for(int p = 0; p < gpd->GetN(); p++)
    {
        auto theta {gpd->GetPointX(p)};
        auto e {gpd->GetPointY(p)};
        if((*minTheta <= theta && theta <= *maxTheta) && (*minE <= e && e <= *maxE))
        {
            auto ex {dt.ReconstructExcitationEnergy(e, theta * TMath::DegToRad())};
            hEx->Fill(ex);
        }
    }

    hEx->DrawClone();
}

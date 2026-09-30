#include "ActDecayGenerator.h"
#include "ActParticle.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"

void test()
{
    auto k35 = ActPhysics::Particle("35K");
    k35.SetEx(0.09);

    ActSim::DecayGenerator decay {k35, "34Ar", "p"};


    auto* hp {new TH2D {"hp", "Proton from decay;#theta proton;T proton", 300, 0, 180, 300, 0, 50}};
    for(int i = 0; i < 1000; i++)
    {
        decay.SetDecay(40, 20 * TMath::DegToRad(), 0);
        decay.Generate();
        auto p {decay.GetLorentzVector(1)};
        auto kinetic = p->E() - decay.GetFinalMass(1);
        auto theta {p->Theta()};
        hp->Fill(theta * TMath::RadToDeg(), kinetic);
    }

    hp->Draw("colz");
}

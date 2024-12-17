#include "ActMergerData.h"
#include "ActModularData.h"
#include "ActSilSpecs.h"
#include "ActTPCData.h"

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TChain.h"

#include "Math/Point3Dfwd.h"

#include <iostream>

void SPSide()
{
    // Read one tree
    auto* chain {new TChain {"ACTAR_Merged"}};
    chain->Add("../../RootFiles/Corrector/Corrected_Run_0155.root");
    chain->AddFriend("ACTAR_Data", "../../RootFiles/Data/Data_Run_0155.root");
    chain->AddFriend("ACTAR_Filter", "../../RootFiles/Filter/Filter_Run_0155.root");


    // Read specs
    ActPhysics::SilSpecs specs;
    specs.ReadFile("../../configs/detailedSilicons.conf");

    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {*chain};

    // Build SPS check
    auto def {
        df.Filter([](ActRoot::ModularData& mod) { return mod.Get("GATCONF") == 8; }, {"ModularData"})
            .Define("SPRaw",
                    [&](ActRoot::MergerData& merger, ActRoot::TPCData& tpc)
                    {
                        ROOT::Math::XYZPointF sp {-1, -1, -1};
                        if(merger.fLightIdx != -1)
                        {
                            auto& line {tpc.fClusters.at(merger.fLightIdx).GetLine()};
                            sp =
                                specs.GetLayer("l0").GetSiliconPointOfTrack(line.GetPoint(), line.GetDirection()).first;
                        }
                        return sp;
                    },
                    {"MergerData", "TPCData"})
            .Define("Quotient", [](ROOT::Math::XYZPointF& r, ROOT::Math::XYZPointF& p)
                    { return ROOT::Math::XYZPointF {p.X() / r.X(), p.Y() / r.Y(), p.Z() / r.Z()}; }, {"SPRaw", "fSP"})};


    auto hxy {def.Define("x", "Quotient.X()")
                 .Define("y", "Quotient.Y()")
                 .Histo2D({"hXY", "XY factor;X;Y", 50, 1.5, 4, 50, 1.5, 4}, "x", "y")};
    auto hz {def.Define("z", "Quotient.Z()").Histo1D({"hz", "Z factor", 200, 1.5, 4}, "z")};
    // Draw
    auto* c0 {new TCanvas {"c0", "Debug SP"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hxy->DrawClone("colz");
    c0->cd(2);
    hz->DrawClone();
}

#include "ActCutsManager.h"
#include "ActDataManager.h"
#include "ActKinematics.h"
#include "ActMergerData.h"
#include "ActParticle.h"
#include "ActSRIM.h"
#include "ActSilMatrix.h"

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TF1.h"
#include "THStack.h"
#include "TString.h"

#include <iostream>
#include <string>
#include <vector>

#include "../../PostAnalysis/Gates.cxx"
#include "../../PostAnalysis/HistConfig.h"
#include "../../PostAnalysis/Utils.cxx"

void run(bool write = false)
{
    // Read silicon matrix
    auto* sm {E796Utils::GetAntiVetoMatrix()};
    auto x3 {sm->GetSil(3)->GetPointX(2)};
    auto x4 {sm->GetSil(4)->GetPointX(2)};

    // And now vary inner edges of silicons 3 and 4
    std::vector<ActPhysics::SilMatrix*> sms;
    for(double s = 0; s < 50; s += 5)
    {
        auto* cl {sm->Clone()};
        // 3
        auto* g3 {cl->GetSil(3)};
        g3->SetPointX(2, x3 + s);
        g3->SetPointX(3, x3 + s);
        // g3->SetPointX(0, x3 - s);
        // g3->SetPointX(1, x3 - s);
        // g3->SetPointX(4, x3 - s);
        // 4
        auto* g4 {cl->GetSil(4)};
        g4->SetPointX(2, x4 + s);
        g4->SetPointX(3, x4 + s);

        // Style
        cl->SetSyle(false);
        // Push back
        sms.push_back(cl);
    }

    if(write)
    {
        // Read data
        ActRoot::DataManager datman {"../../configs/data.conf"};
        auto chain {datman.GetChain()};
        // Apply cuts in f0, RP.X() and PID
        ActRoot::CutsManager<int> cut;
        cut.ReadCut(0, "../../PostAnalysis/Cuts/LightPID/pid_3H.root");
        ROOT::EnableImplicitMT();
        ROOT::RDataFrame df {*chain};
        auto gated {df.Filter(
            [&](ActRoot::MergerData& d)
            {
                // base cut on silicon and rp.x
                auto base {E796Gates::front0(d) && E796Gates::rp(d.fRP.X())};
                if(!base)
                    return base;
                // And PID
                return cut.IsInside(0, d.fSilEs.front(), d.fQave);
            },
            {"MergerData"})};
        // Init SRIM
        std::string beam {"20O"};
        std::string target {"2H"};
        std::string light {"3H"};
        auto* srim {new ActPhysics::SRIM};
        srim->ReadTable(
            light,
            TString::Format("/media/Data/E796v2/Calibrations/SRIMData/raw/%s_952mb_mixture.txt", light.c_str()).Data());
        srim->ReadTable(
            beam,
            TString::Format("/media/Data/E796v2/Calibrations/SRIMData/raw/%s_952mb_mixture.txt", beam.c_str()).Data());

        // Build energy at vertex
        auto def {gated.Define("EVertex", [&](const ActRoot::MergerData& d)
                               { return srim->EvalInitialEnergy(light, d.fSilEs.front(), d.fTrackLength); },
                               {"MergerData"})};

        // Init particles
        ActPhysics::Particle pb {beam};
        ActPhysics::Particle pt {target};
        ActPhysics::Particle pl {light};
        // Build beam energy
        def = def.Define("EBeam", [&](const ActRoot::MergerData& d)
                         { return srim->Slow(beam, 35 * pb.GetAMU(), d.fRP.X()); }, {"MergerData"});

        ActPhysics::Kinematics kin {pb, pt, pl, 35 * pb.GetAMU()};
        // Vector of kinematics as one object is needed per
        // processing slot (since we are changing EBeam in each entry)
        std::vector<ActPhysics::Kinematics> vkins {def.GetNSlots()};
        for(auto& k : vkins)
            k = kin;
        def = def.DefineSlot("Ex",
                             [&](unsigned int slot, const ActRoot::MergerData& d, double EVertex, double EBeam)
                             {
                                 vkins[slot].SetBeamEnergy(EBeam);
                                 return vkins[slot].ReconstructExcitationEnergy(EVertex,
                                                                                d.fThetaLight * TMath::DegToRad());
                             },
                             {"MergerData", "EVertex", "EBeam"});
        // And finally gate on gs
        auto hEx {def.Histo1D(HistConfig::Ex, "Ex")};
        // Fit
        hEx->Fit("gaus", "0RQ+", "", -1.5, 1.5);
        auto* func {hEx->GetFunction("gaus")};
        func->ResetBit(TF1::kNotDraw);
        auto sigma {func->GetParameter("Sigma")};
        auto fin {def.Filter([&](double ex) { return std::abs(ex - 0) <= 3 * sigma; }, {"Ex"})};
        fin.Snapshot("Gated", "gated.root");
        hEx->DrawClone();
    }
    else
    {
        ROOT::EnableImplicitMT();
        ROOT::RDataFrame gated {"Gated", "gated.root"};
        auto hSP {gated.Histo2D(HistConfig::SP, "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

        // And then apply sil matrix cut
        std::vector<TH1D*> hsRPx, hsEx;
        for(int i = 0; i < sms.size(); i++)
        {
            std::cout << "i : " << i << '\n';
            // Apply
            auto node {gated.Filter([i, &sms](ActRoot::MergerData& d)
                                    { return sms[i]->IsInside(d.fSilNs.front(), d.fSP.Y(), d.fSP.Z()); },
                                    {"MergerData"})};
            // Book histogram
            auto hRPx {node.Histo1D(HistConfig::RPx, "fRP.fCoordinates.fX")};
            hRPx->SetTitle(TString::Format("Cut %d", i));

            // Push back and clone
            hsRPx.push_back((TH1D*)hRPx->Clone());
        }

        // Draw
        auto* c0 {new TCanvas {"c0", "Sil matrices impact"}};
        c0->DivideSquare(4);
        c0->cd(1);
        sm->Draw(false);
        for(auto& m : sms)
            m->Draw();
        // Stack
        auto* stack {new THStack};
        stack->SetTitle("Comparison;RP.X() [mm];Counts");
        for(auto& h : hsRPx)
        {
            h->Scale(1. / h->Integral());
            stack->Add(h, "hist");
        }
        c0->cd(2);
        stack->Draw("nostack plc pmc");
        c0->cd(3);
        hSP->DrawClone("colz");
        sm->DrawClone();
    }
}

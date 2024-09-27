#include "ActSRIM.h"

#include "Rtypes.h"

#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TMath.h"
#include "TRandom.h"
#include "TString.h"

#include <cmath>
#include <cstdlib>
#include <vector>

#include "/media/miguel/FICA_4/Juan/Asimp/ACTAR_SimulationFunctions.C"

void srim()
{
    // Test srim is working fine
    ActPhysics::SRIM srim;
    // srim.SetUseSpline(true);
    srim.ReadTable("light", "../../Calibrations/SRIMData/raw/3H_952mb_mixture.txt");

    auto* g {new TGraphErrors};
    g->SetTitle("Straggling;E_{ini} [MeV];Straggling [MeV]");
    auto* ges {new TGraphErrors};
    ges->SetTitle(";E_{ini} [MeV];E_{res} [MeV]");
    auto* gesg {new TGraphErrors};
    gesg->SetTitle(";E_{ini} [MeV];E_{res} [MeV]");
    auto* gsini {new TGraphErrors};
    gsini->SetTitle(";E_{ini} [MeV];Straggling [mm]");
    auto* gsend {new TGraphErrors};
    gsend->SetTitle(";E_{ini} [MeV];Straggling [mm]");
    auto* gsus {new TGraphErrors};
    gsus->SetTitle(";E_{ini} [MeV];Straggling [mm]");
    double start {5};
    double end {100};
    double step {1};
    double thick {100}; // mm
    int iter {10000};
    gRandom->SetSeed();
    for(double e = start; e <= end; e += step)
    {
        std::vector<double> diffs, es, esg, straggs, straggsend, us;
        for(int i = 0; i < iter; i++)
        {
            // Eval with straggling
            auto strag {srim.SlowWithStraggling("light", e, thick)};
            auto nostrag {srim.Slow("light", e, thick)};
            auto diff {std::abs(strag - nostrag)};
            es.push_back(nostrag);
            diffs.push_back(diff);
            esg.push_back(strag);
            auto straggini {srim.EvalLongStraggling("light", srim.EvalRange("light", e))};
            auto straggend {srim.EvalLongStraggling("light", srim.EvalRange("light", e) - thick)};
            straggs.push_back(straggini);
            straggsend.push_back(straggend);
            us.push_back(std::sqrt(straggini * straggini - straggend * straggend));
        }
        g->SetPoint(g->GetN(), e, TMath::Mean(diffs.begin(), diffs.end()));
        ges->SetPoint(ges->GetN(), e, TMath::Mean(es.begin(), es.end()));
        gesg->SetPoint(gesg->GetN(), e, TMath::Mean(esg.begin(), esg.end()));
        gsini->SetPoint(gsini->GetN(), e, TMath::Mean(straggs.begin(), straggs.end()));
        gsend->SetPoint(gsend->GetN(), e, TMath::Mean(straggsend.begin(), straggsend.end()));
        gsus->SetPoint(gsus->GetN(), e, TMath::Mean(us.begin(), us.end()));
    }

    auto* c0 {new TCanvas {"c0", "Check srim canvas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    g->Draw("apl");
    c0->cd(2);
    ges->Draw("apl");
    gesg->SetLineColor(kMagenta);
    gesg->Draw("pl");
    c0->cd(3);
    gsini->Draw("apl");
    gsend->SetLineColor(kMagenta);
    gsend->Draw("pl");
    c0->cd(4);
    gsus->Draw("apl");


    // With Juans code
    TString Filename_ELoss {"/media/miguel/FICA_4/Juan/Asimp/SRIMData/3H_in_952mb_mixture.txt"};
    int N_table_SRIM {};
    N_table_SRIM = Get_Lenght_SRIM_tables(Filename_ELoss);
    double E_dearray[N_table_SRIM], R_dearray[N_table_SRIM], uR_dearray[N_table_SRIM];
    Get_EandR_uR_from_SRIM_tables(Filename_ELoss, N_table_SRIM, E_dearray, R_dearray, uR_dearray);

    auto* gjuan {new TGraphErrors};
    for(double e = start; e <= end; e += step)
    {

        auto Ri = GetRange_fromE(e, N_table_SRIM, E_dearray, R_dearray); // initial range
        auto Rf = Ri - thick;                                            // Final Range

        auto uRi = GetuRange_fromR(Ri, N_table_SRIM, R_dearray, uR_dearray); // initial range stragg
        auto uRf = GetuRange_fromR(Rf, N_table_SRIM, R_dearray, uR_dearray); // final range stragg

        auto e_uR = sqrt(pow(uRi, 2) - pow(uRf, 2)); // effective range stragg
        gjuan->SetPoint(gjuan->GetN(), e, e_uR);
    }

    gjuan->SetLineColor(kGreen);
    gjuan->Draw("lp");
}

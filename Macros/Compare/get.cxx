#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "TCanvas.h"
#include "TChain.h"

#include <iostream>

#include "../../PostAnalysis/HistConfig.h"

// Macro to compare with Juans results
void get()
{
    // Read data
    auto* chain {new TChain {"mini_tree"}};
    chain->Add("/media/miguel/FICA_4/Juan/PostAnalysis2/"
               "Jan23_V2_Analysis_E796_Merge_minitree_analysis_runs_155_to_180_Binary_Date_2023_1_9_Time_21_18.root");


    // Histograms
    auto hPID {HistConfig::PID.GetHistogram()};

    // Read
    double esil0[11] {};
    double esil1[11] {};
    double qave {};
    int gatconf {};
    chain->SetBranchAddress("E_Si0_cal[11]", &esil0);
    chain->SetBranchAddress("E_Si1_cal[11]", &esil1);
    chain->SetBranchAddress("Qave_actar_light", &qave);
    chain->SetBranchAddress("GATCONF", &gatconf);

    // Entries
    auto nentries {chain->GetEntries()};
    for(int i = 0; i < nentries; i++)
    {
        chain->GetEntry(i);
        // Vector checks
        if(gatconf == 4)
        {
            ROOT::RVecD v0(esil0, esil0 + 11);
            auto v0gated {v0[v0 > 0.5]};
            ROOT::RVecD v1(esil1, esil1 + 11);
            auto v1gated {v1[v1 > 0.5]};
            std::cout << "=======" << '\n';
            for(const auto& e : v0gated)
                std::cout << e << '\n';
            std::cout << "---" << '\n';
            for(const auto& e : v1gated)
                std::cout << e << '\n';
            if(v0gated.size() > 0 && v1gated.size() == 0)
            {
                auto sile {v0gated.front()};
                hPID->Fill(sile, qave);
            }
        }
    }


    // Draw
    auto* c0 {new TCanvas {"c0", "Comparison canvas"}};
    c0->DivideSquare(2);
    c0->cd(1);
    hPID->DrawClone("colz");
}

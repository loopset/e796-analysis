#include "ActCalibrationManager.h"
#include "ActTPCLegacyData.h"
#include "ActTPCParameters.h"

#include "TCanvas.h"
#include "TChain.h"
#include "TH2.h"
#include "TString.h"
#include "TStyle.h"

#include <iostream>

void FillHistogram(ActRoot::CalibrationManager* calman, ActRoot::TPCParameters* pars, MEventReduced* evt, TH2D* h,
                   bool isMatched)
{

    // iterate over hits
    for(int it = 0, size = evt->CoboAsad.size(); it < size; it++)
    {
        int co = evt->CoboAsad[it].globalchannelid >> 11;
        int as = (evt->CoboAsad[it].globalchannelid - (co << 11)) >> 9;
        int ag = (evt->CoboAsad[it].globalchannelid - (co << 11) - (as << 9)) >> 7;
        int ch = evt->CoboAsad[it].globalchannelid - (co << 11) - (as << 9) - (ag << 7);
        int where = co * pars->GetNBASAD() * pars->GetNBAGET() * pars->GetNBCHANNEL() +
                    as * pars->GetNBAGET() * pars->GetNBCHANNEL() + ag * pars->GetNBCHANNEL() + ch;

        if((co != 31) && (co != 16))
        {
            auto xval {calman->ApplyLookUp(where, 4)};
            auto yval {calman->ApplyLookUp(where, 5)};
            for(int hit = 0, otherSize = evt->CoboAsad[it].peakheight.size(); hit < otherSize; hit++)
            {
                if((yval != -1) && (xval != -1))
                {
                    double z_position {evt->CoboAsad[it].peaktime[hit]};
                    if(z_position > 0.)
                    {
                        auto Qiaux {evt->CoboAsad[it].peakheight[hit]};
                        // Fill histogram
                        if(isMatched)
                            Qiaux = calman->ApplyPadAlignment(where, Qiaux);
                        // Fill histogram

                        if(Qiaux >= 500) // delete baseline
                        {
                            h->Fill(where, Qiaux);
                        }
                    }
                }
            }
        }
    }
}

void ReadGainMatching(bool isMatched = true)
{

    // Get the data into TChain
    auto chain {new TChain("ACTAR_TTree")};
    std::vector<int> runs {113, 114, 115, 116, 117, 118, 119, 120, 121};
    for(const auto& run : runs)
    {
        chain->Add(TString::Format("../../RootFiles/Raw/Tree_Run_%04d_Merged.root", run));
    }

    // Set the parameters
    ActRoot::TPCParameters tpc {"Actar"};
    // Calibration manager
    ActRoot::CalibrationManager calman {};
    calman.ReadLookUpTable("./LT.dat");
    if(isMatched)
        calman.ReadPadAlign("./Outputs/gain_matching_e796_thesis.dat");

    // Create histogram
    auto* h {new TH2D {"h", "pads;Channel number;Calibrated charge [u.a.]", 17408, 0, 17408, 800, 0, 5000}};

    // Set MEventReduced
    MEventReduced* evt {new MEventReduced};
    chain->SetBranchAddress("data", &evt); // get the column from the chain that we gonna get data from

    for(long int entry = 0, maxEntry = chain->GetEntries(); entry < maxEntry; entry++)
    {
        std::cout << "\r"
                  << "At percent : " << ((double)entry / maxEntry) * 100 << " %" << std::flush;
        chain->GetEntry(entry); // get  the data from the chain and write it to evt variable
        FillHistogram(&calman, &tpc, evt, h, isMatched);
    }

    // Plot
    gStyle->SetOptStat(0);
    auto* c0 {new TCanvas {"c0", "Gain matching canvas"}};
    h->Draw("colz");

    if(!isMatched)
        h->SaveAs("./Inputs/gain.root");
    else
        h->SaveAs("./Outputs/aligned.root");
}

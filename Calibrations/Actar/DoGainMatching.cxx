#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH2.h"
#include "TSpectrum.h"
#include "TStyle.h"

#include <fstream>
#include <iostream>

std::vector<double> GetPeaks(TH2D* h, int c)
{
    auto bin {h->GetXaxis()->FindBin(c)}; // Get the bin number for the channel c
    auto* proj {h->ProjectionY("proj", bin, bin)};

    auto* spe {new TSpectrum(11)}; // max nums of peaks, is overstimated
    int nPeaks {spe->Search(proj, 2, "nodraw", 0.025)};

    double* xPositions {spe->GetPositionX()}; // We have to sort them

    std::sort(xPositions, xPositions + nPeaks); // std::sort needs a pointer for the begin and end of the vector

    std::vector<double> peaks(xPositions, xPositions + nPeaks); // convert the pointer to a vector to easier use

    double width {40.};
    std::vector<double> meanPeak;

    for(auto& peak : peaks)
    {
        double xmin {peak - width};
        double xmax {peak + width};

        proj->Fit("gaus", "0Q", "", xmin, xmax);
        meanPeak.push_back(proj->GetFunction("gaus")->GetParameter("Mean"));
    }
    delete proj;
    delete spe;

    return meanPeak;
}

void FillGraph(int channel, TGraph* graph, const std::vector<double>& x, const std::vector<double>& y,
               std::ofstream& streamer, TGraph* gcal)
{
    if(x.size() != y.size())
    {
        std::cout << "Vectors x and y have different sizes!" << '\n';
        streamer << 0 << " " << 0 << " " << 0 << std::endl;
        return;
    }
    for(int p = 0; p < x.size(); p++)
    {
        graph->SetPoint(p, x[p], y[p]);
    }

    graph->Fit("pol2", "0Q");
    TF1* f {graph->GetFunction("pol2")};
    if(!f)
    {
        std::cout << "Func doesn't exist" << '\n';
        streamer << 0 << " " << 0 << " " << 0 << std::endl;
    }
    else
    {
        streamer << f->GetParameter(0) << " " << f->GetParameter(1) << " " << f->GetParameter(2) << std::endl;
        // Apply calibrationto first peak
        gcal->AddPoint(channel, f->Eval(x.front()));
    }
}

void DoGainMatching()
{
    auto* file {new TFile {"./Inputs/gain.root"}};
    auto* h {file->Get<TH2D>("h")};

    int channels {17408};

    std::vector<std::vector<double>> peaks;

    for(int c = 0; c < channels; c++)
    {
        peaks.push_back(GetPeaks(h, c));
    }

    int channelRef {8124};

    // Now we have to do the fit Q vs Q ref, we use a TGraph, for filling it need a for loop

    std::ofstream streamer {"./Outputs/gain_matching_e796_thesis.dat"};

    std::vector<TGraph*> gs;
    TGraph* graphParams {new TGraph()};
    int idx {-1};
    for(const auto& peak : peaks)
    {
        idx++;
        if(peak.size() != peaks[channelRef - 1].size())
        {
            std::cout << "Channel : " << idx << '\n';
            for(const auto& val : peak)
            {
                std::cout << "   " << val << ' ';
            }
            continue;
        }
        graphParams->SetPoint(idx, idx, peak[0]);
    }
    auto* gCal {new TGraph};
    gCal->SetTitle("First peak cal;Channel;Charge");
    int channel {0};
    for(const auto& peak : peaks)
    {
        TGraph* graph {new TGraph()};
        FillGraph(channel, graph, peak, peaks[channelRef - 1], streamer, gCal);
        graph->SetMarkerStyle(24);
        gs.push_back(graph);
        channel++;
    }
    streamer.close();

    gStyle->SetOptFit(true);
    auto* c {new TCanvas("c")};
    int chosen {3196};
    gs[chosen]->Draw("ap");
    for(auto* ptr : *(gs[chosen]->GetListOfFunctions()))
        if(ptr)
            ptr->Draw("same");

    auto c1 {new TCanvas("c1")};
    gCal->Draw("ap");
}

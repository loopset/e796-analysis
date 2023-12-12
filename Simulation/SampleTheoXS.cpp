#include "TCanvas.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TRandom.h"
#include "TSpline.h"

#include <fstream>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ActSim
{
    class CrossSectionSampler
    {
    private:
        // Vectors holding raw
        std::vector<double> fXData {};
        std::vector<double> fYData {};
        // Sum of absolute cross section
        double fAbsXS {-1};
        // TSPline3 to sample
        std::vector<double> fCDFData {};
        TSpline3* fCDF {};

    public:
        CrossSectionSampler() = default;
        CrossSectionSampler(const std::string& file);

        void ReadData(const std::string& file);

        void Draw() const;

        double Sample(TRandom* rand = nullptr);

    private:
        void Init();
    };
} // namespace ActSim

ActSim::CrossSectionSampler::CrossSectionSampler(const std::string& file)
{
    ReadData(file);
}

void ActSim::CrossSectionSampler::ReadData(const std::string& file)
{
    std::ifstream streamer {file};
    if(!streamer)
        throw std::runtime_error("CrossSectionSampler::ReadData():  could not open input file named " + file);
    double x {};
    double y {};
    while(streamer >> x >> y)
    {
        fXData.push_back(x);
        fYData.push_back(y);
    }
    streamer.close();
    // Call to Init process
    Init();
}

void ActSim::CrossSectionSampler::Init()
{
    // Sum vector of Y*s
    fAbsXS = std::reduce(fYData.begin(), fYData.end());
    // Other way
    // fAbsXS = 0;
    // for(const auto& xs : fYData)
    //     fAbsXS += xs;
    // And build CDF!
    for(int i = 0; i < fYData.size(); i++)
    {
        double acc {};
        for(int j = 0; j <= i; j++)
            acc += fYData[j];
        acc /= fAbsXS;
        fCDFData.push_back(acc);
    }
    // Convert to Spline
    fCDF = new TSpline3 {"fCDF", &(fCDFData[0]), &(fXData[0]), (int)fCDFData.size(), "b2,e2", 0, 0};
}

void ActSim::CrossSectionSampler::Draw() const
{
    auto c {new TCanvas};
    fCDF->SetLineWidth(2);
    fCDF->SetLineColor(kRed);
    fCDF->Draw();
}

double ActSim::CrossSectionSampler::Sample(TRandom* rand)
{
    double r {};
    if(rand)
        r = rand->Uniform();
    else
        r = gRandom->Uniform();
    return fCDF->Eval(r);
}
// The variable spline has to exist during all
// execution time because once plotted, TCanvas requires it to be available
TSpline3* spline {};

void SampleTheoXS()
{
    ActSim::CrossSectionSampler xs {"./Inputs/TheoXS/5.5MeV/angs12nospin.dat"};
    xs.Draw();

    // // 1-> Read TGraphErrors
    // auto* g {new TGraphErrors("./Inputs/TheoXS/5.5MeV/angp32nospin.dat", "%lg %lg")};
    // // 2-> Convert to TSpline: interpolation class
    // spline = new TSpline3("spline", g, "b2,e2", 0, 0);
    // // 3-> Get TF1: use spline as input for a general TF in 1 dim
    // // Using a LAMBDA function
    // auto* func {new TF1(
    //     "func", [&](double* x, double* p) { return spline->Eval(x[0]); }, 0, 180, 0)};
    // func->SetTitle("Diff xs;#theta_{CM} [#circ];Diff xs");
    // func->SetLineColor(kPink);
    // func->SetLineWidth(2);
    //
    // Get random from TF1!!
    auto* hist {new TH1F("hist", "Sampled histogram;#theta_{CM} [deg]", 150, 0, 180)};
    for(int i = 0; i < 10000; i++)
    {
        // auto sampled {func->GetRandom()};
        auto sampled {xs.Sample()};
        hist->Fill(sampled);
    }
    //
    // plotting
    // auto* c1 {new TCanvas("c1", "Canvas")};
    // hist->Draw();
    // c1->DivideSquare(2);
    // c1->cd(1);
    // xs.Draw();
    // c1->cd(2);
    // hist->Draw();
}

#include "ActKinematics.h"

#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDFHelpers.hxx"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RResultHandle.hxx"
#include "ROOT/RResultPtr.hxx"

#include "TCanvas.h"
#include "TH2.h"
#include "TROOT.h"
#include "TString.h"
#include "TSystemDirectory.h"

#include "HistConfig.h"

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

std::string GetParticleName(const std::string& str, const std::string& what)
{
    auto base {str.find(what)};
    auto init {str.find_first_of('_', base) + 1};
    auto end {str.find_first_of('_', init)};
    return str.substr(init, (end - init));
}

struct Signature
{
    std::string beam {};
    std::string target {};
    std::string light {};
};

Signature ExtractName(const std::string& file)
{
    auto beam {GetParticleName(file, "beam")};
    auto target {GetParticleName(file, "target")};
    auto light {GetParticleName(file, "light")};
    return {beam, target, light};
}

void Plotter()
{
    ROOT::EnableImplicitMT();

    // List all files
    TString dirname {"/media/Data/E796v2/PostAnalysis/RootFiles/Pipe2/"};
    TString ext {".root"};
    TSystemDirectory dir {dirname, dirname};
    std::vector<ROOT::RDF::RNode> dfs;
    std::vector<Signature> signatures;
    for(auto* file : *dir.GetListOfFiles())
    {
        TString fname {file->GetName()};
        if(file->IsFolder() && !fname.EndsWith(ext))
            continue;
        std::cout << "Name : " << fname << '\n';
        dfs.push_back(ROOT::RDataFrame {"Final_Tree", (dirname + fname)});
        signatures.push_back(ExtractName(fname.Data()));
    }

    // Book histograms
    std::vector<ROOT::RDF::RResultPtr<TH2D>> hsKin;
    std::vector<ROOT::RDF::RResultPtr<TH1D>> hsEx;
    int counter {-1};
    for(auto& df : dfs)
    {
        counter++;
        auto [b, t, l] {signatures[counter]};
        auto sig {TString::Format("%s(%s, %s)", b.c_str(), t.c_str(), l.c_str())};
        hsKin.emplace_back(
            df.Histo2D(HistConfig::ChangeTitle(HistConfig::Kin, "Kinematics " + sig), "fThetaLight", "EVertex"));
        hsEx.emplace_back(df.Histo1D(HistConfig::ChangeTitle(HistConfig::Ex, "Ex " + sig), "Ex"));
    }

    // Run
    std::vector<ROOT::RDF::RResultHandle> runs;
    for(auto& h : hsKin)
        runs.push_back(h);
    ROOT::RDF::RunGraphs(runs);

    // plotting
    std::vector<TCanvas*> cs {dfs.size()};
    for(int c = 0; c < cs.size(); c++)
    {
        cs[c] = new TCanvas(TString::Format("c%d", c), TString::Format("Canvas %d", c));
        cs[c]->DivideSquare(2);
        cs[c]->cd(1);
        hsKin[c]->DrawClone("colz");
        cs[c]->cd(2);
        hsEx[c]->DrawClone();
    }
}

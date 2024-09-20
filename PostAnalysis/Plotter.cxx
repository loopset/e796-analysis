#ifndef Plotter_cxx
#define Plotter_cxx
#include "ActColors.h"
#include "ActKinematics.h"
#include "ActParticle.h"

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
#include <vector>

#include "/media/Data/E796v2/PostAnalysis/Utils.cxx"
#include "/media/Data/E796v2/Selector/Selector.h"

void Plotter(const std::string& beam = "", const std::string& target = "", const std::string& light = "",
             bool isSide = false)
{
    ROOT::EnableImplicitMT();

    std::vector<ROOT::RDF::RNode> dfs;
    std::vector<E796Utils::Signature> signatures;
    // If called from Runner
    if(beam.length() > 0)
    {
        dfs.push_back(
            ROOT::RDataFrame {"Sel_Tree", E796Utils::GetFile(3, beam, target, light, isSide, gSelector->GetFlag())});
        signatures.push_back({beam, target, light, isSide});
    }
    else
    {
        std::cout << BOLDRED << "Do not use Plotter in this way because you're plotting TTrees at Pipe2 level" << RESET
                  << '\n';
        // List all files
        TString dirname {"/media/Data/E796v2/PostAnalysis/RootFiles/Pipe2/"};
        TString ext {".root"};
        TSystemDirectory dir {dirname, dirname};
        for(auto* file : *dir.GetListOfFiles())
        {
            TString fname {file->GetName()};
            if(file->IsFolder() && !fname.EndsWith(ext))
                continue;
            std::cout << "Name : " << fname << '\n';
            dfs.push_back(ROOT::RDataFrame {"Final_Tree", (dirname + fname)});
            signatures.push_back(E796Utils::ExtractSignature(fname.Data()));
        }
    }

    // Book histograms
    std::vector<ROOT::RDF::RResultPtr<TH2D>> hsKin, hsSP, hsExZ;
    std::vector<ROOT::RDF::RResultPtr<TH1D>> hsEx;
    std::vector<ActPhysics::Kinematics> vkins;
    int counter {-1};
    for(auto& df : dfs)
    {
        counter++;
        auto [b, t, l, isEl] {signatures[counter]};
        ActPhysics::Particle beam {b};
        vkins.push_back(ActPhysics::Kinematics {b, t, l, 35 * beam.GetAMU(), 0});
        auto sig {TString::Format("%s(%s, %s)", b.c_str(), t.c_str(), l.c_str())};
        hsKin.push_back(
            df.Histo2D(HistConfig::ChangeTitle((isEl) ? HistConfig::KinEl : HistConfig::Kin, "Kinematics " + sig),
                       (isEl) ? "fThetaLegacy" : "fThetaLight", "EVertex"));
        hsSP.push_back(df.Histo2D(HistConfig::ChangeTitle(HistConfig::SP, "SP " + sig),
                                  (isEl) ? "fSP.fCoordinates.fX" : "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ"));
        hsEx.push_back(df.Histo1D(HistConfig::ChangeTitle(HistConfig::Ex, "Ex " + sig), "Ex"));
        hsExZ.push_back(df.Histo2D(HistConfig::ExZ, "fSP.fCoordinates.fZ", "Ex"));
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
        cs[c]->DivideSquare(4);
        cs[c]->cd(1);
        hsKin[c]->DrawClone("colz");
        vkins[c].GetKinematicLine3()->Draw("same");
        cs[c]->cd(2);
        hsEx[c]->DrawClone();
        cs[c]->cd(3);
        hsSP[c]->DrawClone("colz");
        if(!signatures[c].isSide)
        {
            E796Utils::GetEffSilMatrix(signatures[c].light)->Draw();
        }
        cs[c]->cd(4);
        hsExZ[c]->DrawClone("colz");
    }
}
#endif

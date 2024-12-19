#include "ActSilMatrix.h"

#include "TAttLine.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
#include "TGraphErrors.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
void PlotAll()
{
    std::vector<std::string> labels {"Veto", "AntiVeto", "Side"};
    std::vector<std::string> files {"./Outputs/veto_matrix.root", "./Outputs/antiveto_matrix.root",
                                    "./Outputs/side_matrix.root"};
    std::vector<std::string> oldfiles {"./Outputs/Dec24/veto_matrix.root", "./Outputs/Dec24/antiveto_matrix.root",
                                       "./Outputs/Dec24/side_matrix.root"};

    // Read
    std::vector<ActPhysics::SilMatrix*> sms, old, clones;
    for(const auto& file : files)
    {
        auto f {std::make_unique<TFile>(file.c_str())};
        auto sm {f->Get<ActPhysics::SilMatrix>("silMatrix")};
        if(!sm)
            throw std::runtime_error("Cannot open SM in " + file);
        sms.push_back(sm);
        auto clone {f->Get<ActPhysics::SilMatrix>("silMatrix")};
        clones.push_back(clone);
    }
    for(const auto& file : oldfiles)
    {
        auto f {std::make_unique<TFile>(file.c_str())};
        auto sm {f->Get<ActPhysics::SilMatrix>("silMatrix")};
        if(!sm)
            throw std::runtime_error("Cannot open SM in " + file);
        old.push_back(sm);
    }

    auto* gHeight {new TGraphErrors};
    gHeight->SetMarkerStyle(24);
    auto& side {sms[2]};
    for(const auto& [idx, _] : side->GetGraphs())
    {
        std::cout << idx << '\n';
        if(!(idx == 1 || idx == 4 || idx == 7))
            continue;
        auto centre {side->GetCentre(idx)};
        auto height {side->GetHeight(idx)};
        gHeight->AddPoint(idx, height);
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "SM comparison"}};
    c0->DivideSquare(2);
    c0->cd(1);
    sms[0]->SetSyle(false);
    sms[0]->Draw(false);
    sms[1]->SetSyle(true, kDashed, 2, 3003);
    sms[1]->Draw();
    c0->cd(2);
    sms[2]->Draw(false, "X [mm]");

    auto* c1 {new TCanvas {"c1", "New/Old comparison"}};
    c1->DivideSquare(4);
    for(int i = 0; i < clones.size(); i++)
    {
        c1->cd(i + 1);
        clones[i]->Draw(false);
        old[i]->SetSyle(false, kDashed);
        old[i]->Draw();
    }

    auto* c2 {new TCanvas {"c2", "Size plot with axis"}};
    gHeight->Draw("ap");
}

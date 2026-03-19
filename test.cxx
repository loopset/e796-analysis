#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TChain.h"
#include "TStyle.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

void test()
{
    std::string path {"/media/miguel/bea/"};
    std::vector<std::pair<std::string, std::string>> files {
        {"/media/miguel/bea/r0101_000a_merged.root", "/media/miguel/bea/r0101_000a_merged_align.root"},
        // {"/media/miguel/bea/r0119_000a_merged.root", "/media/miguel/bea/r0119_000a_merged_align.root"}
    };

    auto* chain {new TChain {"AD"}};
    auto* other {new TChain {"nAD"}};
    int idx {0};
    for(const auto& pair : files)
    {
        chain->Add(pair.first.c_str());
        other->Add(pair.second.c_str());
    }
    std::cout<<"Chain entries : "<<chain->GetEntries()<<'\n';
    std::cout<<"Other entries : "<<other->GetEntries()<<'\n';
    chain->AddFriend(other);

    // ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {*chain};
    // df.Describe().Print();
    auto h {df.Histo2D({"hdf", "DF canvas", 750, 0, 500, 1000, 0, 500}, "E_align", "DE_align")};

    gStyle->SetPalette(kBird);
    h->DrawClone("colz");
}

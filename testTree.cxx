#include "TFile.h"
#include "TMacro.h"
#include "TTree.h"

#include <iostream>

void testTree()
{
    // auto* f {new TFile("./Clusters/Clusters_Run_0155.root")};
    // f->ls();
    // auto* t {f->Get<TTree>("ACTAR_Clusters")};
    // t->Print();
    //
    // auto* txt {f->Get<TMacro>("cluster")};
    // txt->Print();

    // Test macro object
    TMacro config {"./configs/e796.detector", "e796 detector"};

    // txt.ReadFile("./configs/e796.detector");
    config.Print();

    // Print names
    std::cout << "TMacro name : " << config.GetName() << '\n';
    std::cout << "TMacro title : " << config.GetTitle() << '\n';
}

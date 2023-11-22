#include "ActDetectorManager.h"
#include "ActInputData.h"
#include "ActModularData.h"
#include "ActOutputData.h"
#include "ActSilData.h"
#include "ActTPCData.h"
#include "ActTPCPhysics.h"

#include "TFile.h"
#include "TTree.h"

#include <iostream>
#include <memory>
void testInput()
{
    for(int i = 0; i < 10; i++)
    {
        std::cout<<"====================================================================="<<'\n';
        std::cout<<"Repetition : "<<i<<'\n';
        // Set input data
        ActRoot::InputData input {"./configs/merger.runs"};

        // Set output data
        ActRoot::OutputData output {input};
        output.ReadConfiguration("./configs/merger.runs");
        // Init detector
        ActRoot::DetectorManager detman;
        detman.ReadConfiguration("./configs/e796.detector");
        for(const auto& run : input.GetTreeList())
        {
            std::cout << "Building event cluster for run " << run << '\n';
            detman.InitializeMergerInput(input.GetTree(run));
            detman.InitializeMergerOutput(output.GetTree(run));
            std::cout << "Entries in run : " << input.GetNEntries(run) << '\n';
            for(int entry = 0; entry < input.GetNEntries(run); entry++)
            {
                std::cout << "\r"
                          << "At entry : " << entry << std::flush;
                input.GetEntry(run, entry);
                detman.BuildEventMerger(run, entry);
                output.Fill(run);
            }
            input.Close(run);
            output.Close(run);
            std::cout << '\n';
        }
    }
}

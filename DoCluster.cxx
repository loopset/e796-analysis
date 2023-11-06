#include "ActDetectorManager.h"
#include "ActInputData.h"
#include "ActOutputData.h"

#include "TStopwatch.h"

#include <iostream>
#include <ostream>

void DoCluster()
{
    // Set MT or not
    bool enableMT {false};

    // Set input data
    ActRoot::InputData input;
    input.ReadConfiguration("./configs/cluster.runs");

    // Set output data
    ActRoot::OutputData output {input};
    output.ReadConfiguration("./configs/cluster.runs");

    // Run
    if(enableMT)
    {
        ;
    }
    else
    {
        // Init detector
        ActRoot::DetectorManager detman;
        detman.ReadConfiguration("./configs/e796.detector");

        // Timers
        TStopwatch timer {};
        timer.Start();
        for(const auto& run : input.GetTreeList())
        {
            std::cout << "Building event cluster for run " << run << '\n';
            detman.InitializeDataInput(input.GetTree(run));
            detman.InitializePhysicsOutput(input.GetTree(run));
            std::cout << "Entries in run : " << input.GetNEntries(run) << '\n';
            for(int entry  = 0; entry < input.GetNEntries(run); entry++)
            {
                std::cout << "\r" << "At entry : "<<entry<<std::flush;
                input.GetEntry(run, entry);
                detman.BuildEventPhysics();
            }
        }
        std::cout<<std::endl;
        timer.Stop();
        timer.Print();
        // Print inner reports
        detman.PrintReports();
    }
}

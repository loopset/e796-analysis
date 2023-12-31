#include "ActDetectorManager.h"
#include "ActInputData.h"
#include "ActMTExecutor.h"
#include "ActOutputData.h"

#include "TStopwatch.h"

#include <iostream>
#include <ostream>

void DoMerge()
{
    // Set MT or not
    bool enableMT {true};

    // Set input data
    ActRoot::InputData input {"./configs/merger.runs"};

    // Set output data
    ActRoot::OutputData output {input};
    output.ReadConfiguration("./configs/merger.runs");
    output.WriteMetadata("./configs/multistep.conf", "MultiStep");
    output.WriteMetadata("./configs/merger.conf", "Merger");

    // Run
    if(enableMT)
    {
        ActRoot::MTExecutor mt;
        mt.SetInputAndOutput(&input, &output);
        mt.SetDetectorConfig("./configs/e796.detector", "./configs/e796.calibrations");

        mt.BuildEventMerger();
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
            detman.InitInputMerger(input.GetTree(run));
            detman.InitOutputMerger(output.GetTree(run));
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
        std::cout << std::endl;
        timer.Stop();
        timer.Print();
        // Print inner reports
        detman.PrintReports();
    }
}

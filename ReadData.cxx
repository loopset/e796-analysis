#include "ActDetectorManager.h"
#include "ActInputData.h"
#include "ActMTExecutor.h"
#include "ActOutputData.h"

#include "TFile.h"
#include "TStopwatch.h"

#include <ios>
#include <iostream>
void ReadData()
{
    // Set MT or not
    bool enableMT {true};
    std::cout << "Is ActRoot MT enabled? " << std::boolalpha << enableMT << '\n';

    // Set input data
    ActRoot::InputData input;
    input.ReadConfiguration("./configs/e796.runs");
    // Set output data
    ActRoot::OutputData output {input};
    output.ReadConfiguration("./configs/e796.runs");

    if(enableMT)
    {
        ActRoot::MTExecutor mt;
        mt.SetInputAndOutput(&input, &output);
        mt.SetDetectorConfig("./configs/e796.detector", "./configs/e796.calibrations");

        mt.BuildEventData();
    }
    else
    {
        // Set Detector
        ActRoot::DetectorManager detman;
        detman.ReadConfiguration("./configs/e796.detector");
        detman.ReadCalibrations("./configs/e796.calibrations");

        TStopwatch timer {};
        timer.Start();
        // Run!
        for(auto& run : input.GetTreeList())
        {
            std::cout << "Building event data for run " << run << '\n';
            detman.InitializeDataInputRaw(input.GetTree(run), run);
            detman.InitializeDataOutput(output.GetTree(run));
            for(int entry = 0; entry < input.GetTree(run)->GetEntries(); entry++)
            {
                input.GetEntry(run, entry);
                detman.BuildEventData();
                output.Fill(run);
            }
            output.Write(run);
            std::cout << "->Processed events = " << output.GetTree(run)->GetEntries() << '\n';
        }
        timer.Stop();
        timer.Print();
    }
}

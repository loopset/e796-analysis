#include "ActDetectorManager.h"
#include "ActInputData.h"
#include "ActMTExecutor.h"
#include "ActOutputData.h"

#include "TFile.h"
#include "TStopwatch.h"

#include <ios>
#include <iostream>

void ReadWhat(const std::string& outfile, bool enableMT = true)
{
    std::cout << "Is ActRoot MT enabled? " << std::boolalpha << enableMT << '\n';
    bool isCluster {};
    if(outfile.find("cluster") != std::string::npos)
        isCluster = true;
    else
        ; // is data!
    std::cout << "IsCluster ? " << std::boolalpha << isCluster << '\n';

    // Set input data
    ActRoot::InputData input {outfile};
    // Set output data
    ActRoot::OutputData output {input};
    output.ReadConfiguration(outfile);

    if(enableMT)
    {
        ActRoot::MTExecutor mt;
        if(isCluster)
            mt.SetIsCluster();
        else
            mt.SetIsData();
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
            detman.InitInputRaw(input.GetTree(run));
            detman.InitOutputData(output.GetTree(run));
            for(int entry = 0; entry < input.GetTree(run)->GetEntries(); entry++)
            {
                input.GetEntry(run, entry);
                detman.BuildEventData();
                output.Fill(run);
            }
            output.Close(run);
            input.Close(run);
            std::cout << "->Processed events = " << output.GetTree(run)->GetEntries() << '\n';
        }
        timer.Stop();
        timer.Print();
    }
}

void DoRead()
{
    bool enableMT {true};

    // CLUSTER: TPCData
    ReadWhat("./configs/read_clusters.runs", enableMT);
    // DATA : Sil + Modular
    ReadWhat("./configs/read_data.runs", enableMT);
}

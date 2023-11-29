#include "ActDetectorManager.h"
#include "ActInputData.h"
#include "ActOutputData.h"

#include "TStopwatch.h"

#include <iostream>

void DoCorr()
{
    bool enableMT {false};

    ActRoot::InputData input;
    input.SetRunListFrom("./configs/merger.runs");
    input.ReadConfiguration("./configs/corrections.runs");

    ActRoot::OutputData output {input};
    output.ReadConfiguration("./configs/corrections.runs");

    if(enableMT)
        ;
    else
    {
        ActRoot::DetectorManager detman {"./configs/e796.detector"};
        TStopwatch timer {};
        timer.Start();
        // Start loop
        for(const auto& run : input.GetTreeList())
        {
            detman.InitInputCorr(input.GetTree(run));
            detman.InitOutputCorr(output.GetTree(run));
            std::cout << "For run : " << run << " nentries : " << input.GetNEntries(run) << '\n';
            for(int entry = 0; entry < input.GetNEntries(run); entry++)
            {
                std::cout << "\r"
                          << "At entry : " << entry << std::flush;
                input.GetEntry(run, entry);
                detman.BuildEventCorr();
                output.Fill(run);
            }
            std::cout << '\n';
            input.Close(run);
            output.Close(run);
        }
        std::cout << std::endl;
        timer.Stop();
        timer.Print();
    }
}

#include "ActDetectorManager.h"
#include "ActInputData.h"
#include "ActOutputData.h"

#include <algorithm>
#include <iostream>
#include <ostream>
#include <utility>
#include <vector>

std::pair<std::vector<int>, std::vector<int>> thing()
{
    std::vector<int> a {1 ,1 ,1};
    std::vector<int> b {2, 2};
    return std::make_pair(std::move(a), std::move(b));
}

void testDetMan()
{
    ActRoot::InputData input {"../configs/read.runs"};
    ActRoot::OutputData output {input};
    output.ReadConfiguration("../configs/read.runs");

    ActRoot::DetectorManager detman {"../configs/e796.detector"};
    detman.SetRawToDataDetectors({ActRoot::DetectorType::ESilicons, ActRoot::DetectorType::EModular});
    detman.ReadCalibrations("../configs/e796.calibrations");

    for(const auto& run : input.GetTreeList())
    {
        detman.InitInputRaw(input.GetTree(run));
        detman.InitOutputData(output.GetTree(run));
        std::cout << "Run : " << run << " with total entries : " << input.GetNEntries(run) << '\n';
        int max {100};
        for(int entry = 0; entry < max; entry++)
        {
            input.GetEntry(run, entry);
            detman.BuildEventData();
            output.Fill(run);
            std::cout << "\r"
                      << "At entry : " << entry << std::flush;
        }
        input.Close(run);
        output.Close(run);
        std::cout<<std::endl;
    }

    const auto& [a,b] = thing();
    for(auto& e : a)
        std::cout<<"a element : "<<e<<'\n';
}

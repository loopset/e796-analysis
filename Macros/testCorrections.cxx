#include "ActJoinData.h"
#include "ActMergerData.h"
void testCorrections()
{
    ActRoot::JoinData data {"../configs/merger.runs"};

    ActRoot::MergerData* md {};
    data->SetBranchAddress("MergerData", &md);

    for(int entry = 0; entry < data->GetEntries(); entry++)
    {
        data->GetEntry(entry);
    }
}

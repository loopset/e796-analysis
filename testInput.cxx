#include "ActDetectorManager.h"
#include "ActInputData.h"
#include "ActModularData.h"
#include "ActOutputData.h"
#include "ActSilData.h"
#include "ActTPCData.h"
#include "ActTPCPhysics.h"

#include "TFile.h"
#include "TH1.h"
#include "TTree.h"

#include <iostream>
#include <memory>
struct S 
{
    TH1F fHist {};

    void Reset(){*this = S{};}
};
void testInput()
{
    S st {};
    for(int i = 0; i < 10; i++)
    {
        auto h {TH1F("hProfile", "Profile", 100, 0, 100)};
        st.fHist = h;

        st.Reset();
    } 
}

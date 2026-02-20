#include "Task0.h"

#include "ActSilData.h"
#include "ActVTask.h"

#include <iostream>



bool ActAlgorithm::Task0::Run()
{
    std::cout << "Pointers : " << '\n';
    std::cout << "MergerData : " << fMergerData << '\n';
    std::cout << "SilData : " << fSilData << '\n';
    std::cout << "SilData mult : " << fSilData->GetMult("f0") << '\n';
    fSilData->Clear();
    std::cout << "SilData mult : " << fSilData->GetMult("f0") << '\n';
    std::cout << "RUNNNNNNNNNNNING" << '\n';
    return true;
}

void ActAlgorithm::Task0::Print()
{
    std::cout << "Tas0 prints!" << '\n';
}

// Create symbol to load class from .so
// extern "C" disables C++ function name mangling
extern "C" ActAlgorithm::Task0* Create()
{
    return new ActAlgorithm::Task0;
}

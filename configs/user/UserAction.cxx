#include "UserAction.h"

#include "ActInputParser.h"
#include "ActTPCData.h"

#include <memory>

void ActAlgorithm::UserAction::UserAction::ReadConfiguration(std::shared_ptr<ActRoot::InputBlock> block)
{
    std::cout << "Ola : " << block->GetString("Ola") << '\n';
}

void ActAlgorithm::UserAction::Run()
{
    for(const auto& cluster : fTPCData->fClusters)
    {
        cluster.Print();
    }
}

void ActAlgorithm::UserAction::Print() const
{
    std::cout << "Loaded UserAction!" << '\n';
}

// Create symbol to load class from .so
extern "C" ActAlgorithm::UserAction* CreateUserAction()
{
    return new ActAlgorithm::UserAction;
}

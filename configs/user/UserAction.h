#include "ActVAction.h"

namespace ActAlgorithm
{
class UserAction : public VAction
{
public:
    UserAction() : VAction("UserAction") {}

    void ReadConfiguration(std::shared_ptr<ActRoot::InputBlock> block) override;
    void Run() override;
    void Print() const override;
};
} // namespace ActAlgorithm

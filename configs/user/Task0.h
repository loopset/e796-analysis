#ifndef Task0_h
#define Task0_h

#include "ActVTask.h"

#include <map>
#include <string>
#include <vector>

namespace ActAlgorithm
{
class Task0 : public VTask
{
public:
    std::map<int, std::vector<std::pair<std::string, int>>> fMap {
        {45, {{"f0", 5}}}
    };

    Task0() : VTask("Task0") {}

    bool Run() override;
    void Print() override;
};
} // namespace ActAlgorithm

#endif // !Task0_h

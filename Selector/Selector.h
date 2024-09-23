#ifndef Selector_h
#define Selector_h

#include "ActInputParser.h"

#include "TString.h"

#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
namespace E796
{
class Config
{
public:
    std::pair<double, double> fRPx {};
    double fLengthX {};

    void ReadConfig(std::shared_ptr<ActRoot::InputBlock> b);
    void Print() const;
};
class Selector
{
private:
    static Selector* fInstance;
    Selector() {};
    ~Selector() {};
    static std::mutex fMutex;
    // Constructor which reads a config file
    Selector(const std::string& file);

    // Parameters that define the current selection
    std::string fBeam {};
    std::string fTarget {};
    std::string fLight {};

    // Other configurations in the selector
    std::unordered_map<std::string, Config> fConfigs;
    Config* fCurrent {};

    // Current selection
    std::string fFlag {};

public:
    Selector(Selector&) = delete;
    void operator=(const Selector&) = delete;

    static Selector* GetInstance(const std::string& file = "");
    void Print() const;

    // Set configuration
    void SetFlag(const std::string& flag);

    const std::string& GetFlag() const { return fFlag; }
    double GetRPxLow() const { return fCurrent->fRPx.first; }
    double GetRPxUp() const { return fCurrent->fRPx.second; }
    std::pair<double, double> GetRPx() const { return fCurrent->fRPx; }
    double GetLengthX() const { return fCurrent->fLengthX; }

    // Formatting functions
    TString GetAnaFile(int pipe, const std::string& beam, const std::string& target, const std::string& light,
                       bool withFlag = true);
    TString GetSimuFile(const std::string& beam, const std::string& target, const std::string& light, double Ex,
                        int nPS = 0, int pPS = 0);

    // Executing functions
    void RecomputeNormalization() const;

    // Particle names
    const std::string GetBeam() const { return fBeam; }
    const std::string GetTarget() const { return fTarget; }
    const std::string GetLight() const { return fLight; }

private:
    void ReassignNames();
};
} // namespace E796

#define gSelector (E796::Selector::GetInstance("/media/Data/E796v2/Selector/selector.conf"))

#endif

#ifndef Selector_h
#define Selector_h

#include "ActInputParser.h"

#include "TString.h"

#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// forward declaration
class TObject;
class TList;

namespace E796
{
class Config
{
public:
    std::pair<double, double> fRPx {};
    double fLengthX {};
    bool fMaskElSil {};

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

    // Particle information
    std::string fBeam {};
    std::string fTarget {};
    std::string fLight {};

    // List all configurations under the config file
    std::unordered_map<std::string, Config> fConfigs;

    // Current selection
    std::string fFlag {};
    Config* fCurrent {};

public:
    Selector(Selector&) = delete;
    void operator=(const Selector&) = delete;

    static Selector* GetInstance(const std::string& file = "");
    void Print() const;

    // Setters
    void SetFlag(const std::string& flag);
    void SetBeam(const std::string& beam)
    {
        fBeam = beam;
        ReassignNames();
    }
    void SetTarget(const std::string& target)
    {
        fTarget = target;
        ReassignNames();
    }
    void SetLight(const std::string& light)
    {
        fLight = light;
        ReassignNames();
    }

    // Getters
    const std::string& GetFlag() const { return fFlag; }
    double GetRPxLow() const { return fCurrent->fRPx.first; }
    double GetRPxUp() const { return fCurrent->fRPx.second; }
    std::pair<double, double> GetRPx() const { return fCurrent->fRPx; }
    double GetLengthX() const { return fCurrent->fLengthX; }
    bool GetMaskElSil() const { return fCurrent->fMaskElSil; }

    std::vector<std::string> GetFlags() const;

    // Formatting functions
    TString GetAnaFile(int pipe, bool withFlag = true);
    TString GetAnaFile(int pipe, const std::string& beam, const std::string& target, const std::string& light,
                       bool withFlag = true);
    TString GetSimuFile(double Ex, int nPS = 0, int pPS = 0);
    std::vector<std::string> GetSimuFiles(int nPS = 0, int pPS = 0);
    TString GetSimuFile(const std::string& beam, const std::string& target, const std::string& light, double Ex,
                        int nPS = 0, int pPS = 0);
    std::vector<std::string> GetSimuFiles(const std::string& beam, const std::string& target, const std::string& light,
                                          int nPS = 0, int pPS = 0);
    std::string GetApproxSimuFile(const std::string& beam, const std::string& target, const std::string& light,
                                  double Ex, int nPS = 0, int pPS = 0);
    TString GetSigmasFile(const std::string& target, const std::string& light);

    // Executing functions
    void RecomputeNormalization() const;

    // Other functions
    void SendToWebsite(const std::string& file, TObject* o);
    void SendToWebsite(const std::string& file, TList* list);

    // Particle names
    const std::string GetBeam() const { return fBeam; }
    const std::string GetTarget() const { return fTarget; }
    const std::string GetLight() const { return fLight; }
    const bool GetIsElastic() const { return fTarget == fLight; }

private:
    void ReassignNames();
    double GetExFromFileName(const std::string& file);
};
} // namespace E796

#define gSelector (E796::Selector::GetInstance("/media/Data/E796v2/Selector/selector.conf"))

#endif

#ifndef Selector_h
#define Selector_h

#include "TString.h"

#include <mutex>
#include <string>
#include <utility>
namespace E796
{
class Selector
{
private:
    static Selector* fInstance;
    Selector() {};
    ~Selector() {};
    static std::mutex fMutex;
    // Constructor which reads a config file
    Selector(const std::string& file);

    // All components of the configuration

    // All parameters of the selector
    std::string fFlag {};
    std::pair<double, double> fRPx {};
    bool fEnableF07 {};

public:
    Selector(Selector&) = delete;
    void operator=(const Selector&) = delete;

    static Selector* GetInstance(const std::string& file = "");
    void Print() const;

    const std::string& GetFlag() const { return fFlag; }
    const double& GetRPxLow() const { return fRPx.first; }
    const double& GetRPxUp() const { return fRPx.second; }
    const std::pair<double, double> GetRPx() const { return fRPx; }

    // Formatting functions
    TString
    GetAnaFile(int pipe, const std::string& beam, const std::string& target, const std::string& light);
    TString GetSimuFile(const std::string& beam, const std::string& target, const std::string& light, double Ex,
                        int nPS = 0, int pPS = 0);
};
} // namespace E796

#define gSelector (E796::Selector::GetInstance("/media/Data/E796v2/Selector/selector.conf"))

#endif

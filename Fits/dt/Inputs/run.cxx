#include "TSystem.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
void run(const std::string& current_path = "./")
{
    for(const auto& subdir : std::filesystem::recursive_directory_iterator(current_path))
    {
        if(std::filesystem::is_directory(subdir))
        {

            auto path {subdir.path()};
            auto name {path.string()};
            if(name.find("ex_") != std::string::npos)
            {
                // If has fresco.in inside
                auto file {path / "fresco.in"};
                if(std::filesystem::exists(file))
                {
                    std::cout << "Fresco in dir : " << path << '\n';
                    auto pwd {std::filesystem::current_path()};
                    std::filesystem::current_path(path);
                    system("fresco <fresco.in> fresco.out");
                    std::filesystem::current_path(pwd);
                }
            }
        }
    }
}

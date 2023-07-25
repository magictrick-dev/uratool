#include <resourceconfiguration.h>
#include <iostream>
#include <iomanip>
#include <fstream>
using json = nlohmann::json;

RoutineConfiguration::
RoutineConfiguration()
    : _loaded(false)
{ }

void RoutineConfiguration::
parse_configurations()
{

    if (!this->is_loaded())
        return;

    // If the profile we loaded is correctly formatted, a root
    // member called profiles should be present as an array.
    if (this->_profile["profiles"].is_array())
    {

        for (json current_profile : this->_profile["profiles"])
        {
            if (!current_profile.contains("name")               || !current_profile["name"].is_string() ||
                !current_profile.contains("backup_location")    || !current_profile["backup_location"].is_string() ||
                !current_profile.contains("drives")             || !current_profile["drives"].is_array())
            {
                std::cout << "Invalid profile:\n" << current_profile.dump() << std::endl;
                continue;
            }


            std::cout << "Valid profile:\n" << current_profile.dump() << std::endl;

        }

    }

    // Otherwise, we should create one.
    else
    {
        this->_profile["profiles"] = json::array();
    }

}

bool RoutineConfiguration::
load_profile(std::string file_path)
{

    std::ifstream file(file_path);   
    if (file.is_open())
    {
        this->_profile      = json::parse(file);   
        this->_loaded       = true;
        this->_file_path    = file_path;
        this->parse_configurations();
        return true;
    }
    else
    {
        return false;
    }

}

void RoutineConfiguration::
save_profile(std::string file_path)
{
    
    std::ofstream file(file_path);
    if (file.is_open())
    {
        file << std::setw(4) << this->_profile;
    }

}

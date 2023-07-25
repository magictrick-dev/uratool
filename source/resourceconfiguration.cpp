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
    if (this->_profile.contains("profiles")
            && this->_profile["profiles"].is_array())
    {

        // Validate the profile.
        // TODO(Chris): We will need to find a more robust way of handling
        // JSON layout validation.
        for (json current_profile : this->_profile["profiles"])
        {

            if (!current_profile.contains("name")               || !current_profile["name"].is_string() ||
                !current_profile.contains("backup_location")    || !current_profile["backup_location"].is_string() ||
                !current_profile.contains("drives")             || !current_profile["drives"].is_array())
            {
                continue;
            }

            
            Configuration current_config = {};
            current_config.name = current_profile["name"];
            current_config.backup_location = current_profile["backup_location"];
            
            for (json drive : current_profile["drives"])
            {
                if (drive.is_primitive() && drive.is_string())
                {
                    current_config.drives.push_back(drive);
                }
            }
            
            // Print the config.
            std::cout << current_config.name << std::endl;
            std::cout << current_config.backup_location << std::endl;
            std::cout << "Drives:" << std::endl;
            for (std::string a : current_config.drives)
                std::cout << "    " << a << std::endl;

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
        // We don't need json::parse is explosive if there's even a slight
        // error, so we may be better off using json::accept to determine if
        // the file is good before straight up blowing up.
        this->_profile      = json::parse(file);   
        this->_loaded       = true;
        this->_file_path    = file_path;

        // Parse the configuration, whether or not it is valid may be something
        // we should check for...
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

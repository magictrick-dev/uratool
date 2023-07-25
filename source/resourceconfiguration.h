#ifndef URATOOL_RESOURCE_CONFIGURATION_H
#define URATOOL_RESOURCE_CONFIGURATION_H
#include <vendor/jsoncpp/json.hpp>
#include <vector>
#include <string>

// -----------------------------------------------------------------------------
// Routine Configuration & Helpers
// -----------------------------------------------------------------------------
// The routine configuration determines what USBs get backed up to where, defining
// a Cron timing schema that performs the routine at the given interval. The
// routine configuration loads a JSON profile which saves to disk any configuration
// created. These configurations may be modified or removed through the front-end.

struct Configuration
{
    std::string                 name;
    std::string                 backup_location;
    std::vector<std::string>    drives;
    
};

class RoutineConfiguration
{
    public:
        RoutineConfiguration();

        bool        load_profile(std::string file_path);
        void        save_profile(std::string file_path);
        inline bool is_loaded() const { return this->_loaded; }

    protected:
        void        parse_configurations();

    protected:
        nlohmann::json              _profile;
        bool                        _loaded;
        std::string                 _file_path;

        std::vector<Configuration>  _configurations;
};

#endif

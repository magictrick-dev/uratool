#ifndef URATOOL_RESOURCE_CONFIGURATION_H
#define URATOOL_RESOURCE_CONFIGURATION_H
#include <vendor/jsoncpp/json.hpp>
#include <libcron/Cron.h>
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
    std::string                 cron_time;
    std::vector<std::string>    drives;
};

class Routines 
{
    public:
        Routines();

        bool        load_profile(std::string file_path);
        void        save_profile(std::string file_path);
        inline bool is_loaded() const { return this->_loaded; }

        Configuration*      get_config_by_name(std::string config_name);

        void        update();

    protected:
        void        parse_configurations();

        static      void backup_procedure(Configuration*);
        static      void process_backups(const libcron::TaskInformation&);

    protected:
        nlohmann::json              _profile;
        bool                        _loaded;
        std::string                 _file_path;
        std::vector<Configuration>  _configurations;
        libcron::Cron<>              _cron_jobs;
};

#endif

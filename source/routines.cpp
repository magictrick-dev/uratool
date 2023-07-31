#include <routines.h>
#include <application.h>
#include <iostream>
#include <iomanip>
#include <fstream>
using json = nlohmann::json;

Routines::
Routines()
    : _loaded(false)
{ }

void Routines::
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
                !current_profile.contains("cron_schedule")      || !current_profile["cron_schedule"].is_string() ||
                !current_profile.contains("drives")             || !current_profile["drives"].is_array())
            {
                continue;
            }

            // Search for any duplicates, name must unique.
            bool duplicate = false;
            for (Configuration& config : this->_configurations)
            {
                if (config.name == current_profile["name"])
                {
                    duplicate = true;
                    break;
                }
            }

            if (duplicate == true)
                continue; // No duplicates.

            // Otherwise, create the configuration object and store it.
            Configuration current_config = {};
            current_config.name = current_profile["name"];
            current_config.backup_location = current_profile["backup_location"];
            for (json drive : current_profile["drives"])
                if (drive.is_primitive() && drive.is_string())
                    current_config.drives.push_back(drive);

            this->_configurations.push_back(current_config);
        }

    }

    // Otherwise, we should create one.
    else
    {
        this->_profile["profiles"] = json::array();
    }

}

void Routines::
update()
{
    this->_cron_jobs.tick();
}

Configuration* Routines::
get_config_by_name(std::string config_name)
{
 
    // Search for the configuration.
    for (size_t i = 0; i < this->_configurations.size(); ++i)
        if (this->_configurations[i].name == config_name)
            return &this->_configurations[i];

    // If the search failed, then return a null ptr.
    return NULL;

}

void Routines::
process_backups(const libcron::TaskInformation& task_info)
{
    
    // Cast back into child interface since the parent interface
    // doesn't seem to recognize the get_name() method.
    const libcron::Task& task = (const libcron::Task&)task_info;

    // Get the name.
    std::string task_name = task.get_name();

    // Search for the configuration of the routine.
    Routines& application_routines = Application::get_routines();
    Configuration* config = application_routines.get_config_by_name(task_name);

    if (config == NULL)
        return;

    // Perform the backup procedure.
    backup_procedure(config);

    return;
}

void Routines::
backup_procedure(Configuration* config)
{
    
    SCOPE_APPLICATION(application);

    // Test procedure.
    std::stringstream oss;
    oss << "Backup procedure started for " << config->name
        << " at: " << config->backup_location;

    application.print(oss.str());

}

bool Routines::
load_profile(std::string file_path)
{

    SCOPE_APPLICATION(application);

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

        // Once we have the configurations parsed, we should add their schedules
        // into the system. We need to maintain a purge list of configurations
        // that were not successfully added to the routines.
        std::vector<std::string> purge_list;
        for (size_t i = 0; i < this->_configurations.size(); ++i)
        {
            Configuration* current_config = &this->_configurations[i];

            bool schedule_added = this->_cron_jobs.add_schedule(current_config->name,
                    current_config->cron_time, process_backups);
            if (!schedule_added)
                purge_list.push_back(current_config->name);
        }

        // Remove the configurations that were not added to the routine.
        for (size_t i = 0; i < purge_list.size(); ++i)
        {
            std::stringstream oss;
            oss << "Invalid cron schedule for configuration: "
                << purge_list[i];
            application.print(oss.str());
            // TODO(Chris): Implement the purge routine.
        }

        return true;
    }
    else
    {
        return false;
    }

}

void Routines::
save_profile(std::string file_path)
{
    
    std::ofstream file(file_path);
    if (file.is_open())
    {
        file << std::setw(4) << this->_profile;
    }

}

#include <gui_thread.h>
#include <udev_thread.h>
#include <state.h>

#include <ncurses.h>
#include <unistd.h>

#include <sstream>
#include <vector>

GUIThread::
GUIThread()
{ }

GUIThread::
~GUIThread()
{ }

bool GUIThread::
get_runtime_state()
{
	bool current_state;
	pthread_mutex_lock(&this->_m_runtime);
	current_state = _runtime_state;
	pthread_mutex_unlock(&this->_m_runtime);
	return current_state;
}

void GUIThread::
set_runtime_state(bool state)
{
	pthread_mutex_lock(&this->_m_runtime);
	_runtime_state = state;
	pthread_mutex_unlock(&this->_m_runtime);
}

void GUIThread::
print(std::string message)
{
	
	// To be thread safe, we need to ensure that gather a lock here.
	pthread_mutex_lock(&this->_m_print);

	// Messages may contain multiple lines, so split and insert.
	size_t pos = message.find("\n");

	if (pos == std::string::npos)
	{
		this->_insert_output_line(message);
	}
	else
	{
		// Multiline string, print until empty.
		std::string token;
		while ((pos = message.find("\n")) != std::string::npos) {
			token = message.substr(0, pos);
			this->_insert_output_line(token);
			message.erase(0, pos + 1);
		}
		this->_insert_output_line("");
	}

	// Release the lock.
	pthread_mutex_unlock(&this->_m_print);
}

void GUIThread::
_insert_output_line(std::string message)
{
	int next = this->roll_size % ROLL_BUFFER_MAX;
	this->output_rolling_buffer[next] = message;
	this->roll_size++;
	if (this->roll_size >= ROLL_BUFFER_MAX)
		this->roll_offset = (this->roll_size % ROLL_BUFFER_MAX);
}

void GUIThread::
show_output()
{
	// Determine log region.
	int max_size = this->term_height - 2; // Bottom is reserved for commands.
	int printable_size = (max_size < ROLL_BUFFER_MAX) ? max_size : ROLL_BUFFER_MAX;	
	int viewable_size = (this->roll_size < printable_size) ? this->roll_size : printable_size;
	int delta_start = this->roll_size - viewable_size;

	if (delta_start < 0)
		delta_start = 0; // Enforce that the delta start is > 0.
	int output_index = delta_start % ROLL_BUFFER_MAX;
	for (int i = 0; i < viewable_size; ++i)
	{
		printw("%s\n", this->output_rolling_buffer[output_index].c_str());
		output_index = (output_index + 1) % ROLL_BUFFER_MAX;
	}
}

void GUIThread::
process_command(std::string command)
{

    // Split the command by spaces.
    std::vector<std::string> command_splits;
    std::string delimiter = " ";

    size_t pos = 0;
    std::string token;
    while ((pos = command.find(delimiter)) != std::string::npos) {
        token = command.substr(0, pos);
        command_splits.push_back(token);
        command.erase(0, pos + delimiter.length());
    }

    command_splits.push_back(command);

    // Set the primary command.
    std::string primary_command = command_splits[0];

    // -------------------------------------------------------------------------
    // Quit Command
    // -------------------------------------------------------------------------
    if (primary_command == "quit")
    {
        this->set_runtime_state(false); // Exit.
    }

    // -------------------------------------------------------------------------
    // Help Command
    // -------------------------------------------------------------------------
    else if (primary_command == "help")
    {
        this->print("Available commands:");
        this->print("    Type \"help\" to display a list of commands. You are here!");
        this->print("    Type \"quit\" to exit.");
        this->print("    Type \"clear\" to clear the message output.");
        this->print("    Type \"mount [uuid]\" to manually mount a USB drive.");
        this->print("    Type \"unmount [uuid]\" to manually unmount a USB drive.");
        this->print("    Type \"ls\" to list available devices.");
    }

    // -------------------------------------------------------------------------
    // Clear Command
    // -------------------------------------------------------------------------
    else if (primary_command == "clear")
    {
        this->roll_size = 0;
        this->roll_offset = 0;
    }

    // -------------------------------------------------------------------------
    // Mount Command
    // -------------------------------------------------------------------------
    else if (primary_command == "mount")
    {
        if (command_splits.size() >= 2)
        {
            get_state()->udev_thread->lock_storage_devices();
            StorageDevice* device = get_state()->udev_thread->find_device_by_uuid(command_splits[1]);
            if (device != NULL)
            {
                device->mount_device();
            }
            else
            {
                std::stringstream oss;
                oss << "Unable to find device with UUID: " << command_splits[1];
                this->print(oss.str());
            }
            get_state()->udev_thread->unlock_storage_devices();
        }
        else
        {
            this->print("Invalid command format: mount [uuid]");
        }
    }

    // -------------------------------------------------------------------------
    // Unmount Command
    // -------------------------------------------------------------------------
    else if (primary_command == "unmount")
    {
        if (command_splits.size() >= 2)
        {
            get_state()->udev_thread->lock_storage_devices();
            StorageDevice* device = get_state()->udev_thread->find_device_by_uuid(command_splits[1]);
            if (device != NULL)
            {
                device->unmount_device();
            }
            else
            {
                std::stringstream oss;
                oss << "Unable to find device with UUID: " << command_splits[1];
                this->print(oss.str());
            }
            get_state()->udev_thread->unlock_storage_devices();
        }
        else
        {
            this->print("Invalid command format: mount [uuid]");
        }
    }

    // -------------------------------------------------------------------------
    // List Command
    // -------------------------------------------------------------------------
    else if (primary_command == "ls")
    {
        // Create an OSS for displaying the output.
        std::stringstream oss;

        // Loop through the devices and stream them into the oss.
        get_state()->udev_thread->lock_storage_devices();
        std::vector<StorageDevice>* device_list = get_state()->udev_thread->get_device_list();
        if (device_list != NULL)
        {
            this->print("Available devices:");
            for (size_t i = 0; i < device_list->size(); ++i)
            {
                StorageDevice& current_device = device_list->at(i);
                oss << "    " << i + 1 << " " << current_device.get_dev_name() << " w/ UUID: "
                    << current_device.get_uuid() << " with dev-path: "
                    << current_device.get_dev_path();
                this->print(oss.str());
                oss.str("");
            }
        }
        get_state()->udev_thread->unlock_storage_devices();
    }

    // -------------------------------------------------------------------------
    // Uncaught & Default
    // -------------------------------------------------------------------------
    else
    {
        std::stringstream error_out;
        error_out << "Unknown command: " << this->command_buffer;
        this->print(error_out.str());
        this->print("    Type \"help\" to display a complete list of commands.");
        this->print("    Type \"quit\" to exit.");
    }

}

void GUIThread::
main()
{
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	this->roll_size = 0;
	this->roll_offset = 0;

	this->print("Uratool Version 0.2-Aplha by Chris DeJong 2023");
	this->print("    Uratool, USB Replication Automation Tool, is designed");
	this->print("    to make managing operations-critical USB flash drives");
	this->print("    painless and easy.\n");
	this->print("    Type \"help\" to display a list of commands.");
	this->print("    Type \"quit\" to exit.\n");

	int ch = ERR;
	while (this->get_runtime_state())
	{

		// Update the current terminal size.
		getmaxyx(stdscr, this->term_height, this->term_width);
		
		// Clear the screen and show the output.
		erase();
		show_output();

		// Move the cursor to the command location.
		// We also need to refresh the display since 
		move(this->term_height - 1, 0);
		printw(">> %s", this->command_buffer.c_str());
		refresh();

		// Character Input
		if ((ch = getch()) != ERR)
		{

			// Enter means we should process the command.
			if (ch == KEY_ENTER || ch == '\n')
			{

                process_command(this->command_buffer);

				// Clear the command buffer.
				this->command_buffer.clear();

			}

			// Backspace means pop a character.
			else if (ch == KEY_BACKSPACE)
			{
				if (!this->command_buffer.empty())
					this->command_buffer.pop_back();
			}

			else if (ch < 127 && ch > 31)
			{
				this->command_buffer.push_back((char)ch);
			}
		}

		// Otherwise, do some other stuff?
		else
		{
			usleep(8000); // 16ms sleep.
		}

		refresh();
	}

	endwin();

}

void GUIThread::
exit()
{
	this->set_runtime_state(false);
}

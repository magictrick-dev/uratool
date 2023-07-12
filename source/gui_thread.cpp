#include <gui_thread.h>
#include <ncurses.h>
#include <unistd.h>
#include <sstream>

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
main()
{
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	this->roll_size = 0;
	this->roll_offset = 0;

	this->print("Uratool Version 0.1-Aplha by Chris DeJong 2023");
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

				if (this->command_buffer == "quit")
				{
					this->set_runtime_state(false); // Exit.
				}
				else if (this->command_buffer == "help")
				{
					this->print("Available commands:");
					this->print("    Type \"help\" to display a list of commands. You are here!");
					this->print("    Type \"quit\" to exit.");
					this->print("    Type \"clear\" to clear the message output.");
				}
				else if (this->command_buffer == "clear")
				{
					this->roll_size = 0;
					this->roll_offset = 0;
				}
				else
				{
					std::stringstream error_out;
					error_out << "Unknown command: " << this->command_buffer;
					this->print(error_out.str());
					this->print("    Type \"help\" to display a complete list of commands.");
					this->print("    Type \"quit\" to exit.");
				}

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

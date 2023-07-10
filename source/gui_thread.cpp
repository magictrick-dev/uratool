#include <gui_thread.h>
#include <ncurses.h>

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
main()
{
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	int ch = ERR;
	while (get_runtime_state())
	{

		// Character Input
		if ((ch = getch()) != ERR)
		{
			// If it is escape, set GUI runtime to false.
			if (ch == 'q')
			{
				this->set_runtime_state(false);
				continue;
			}

			else if (ch < 127)
			{
				addch(ch);
			}
		}

		// Otherwise, do some other stuff?
		else
		{

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

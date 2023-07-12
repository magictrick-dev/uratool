#ifndef GUI_THREAD_H
#define GUI_THREAD_H
#include <core/threading.h>
#include <string>
#define ROLL_BUFFER_MAX 256

class GUIThread : public Thread
{

	public:
		GUIThread();
		~GUIThread();

		THREAD_MAIN(GUIThread);
		virtual void 	exit();

		bool 			get_runtime_state();
		void 			set_runtime_state(bool state);

		void 			print(std::string);
		void 			show_output();

	protected:
		int term_width;
		int term_height;
		int roll_offset;
		int roll_size;
		std::string output_rolling_buffer[ROLL_BUFFER_MAX];
		std::string command_buffer;

		void 			_insert_output_line(std::string message);

	protected:
		pthread_mutex_t 	_m_print;
		pthread_mutex_t 	_m_runtime;
		bool 				_runtime_state;
};

#endif

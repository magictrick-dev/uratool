#ifndef GUI_THREAD_H
#define GUI_THREAD_H
#include <core/threading.h>

class GUIThread : public Thread
{

	public:
		GUIThread();
		~GUIThread();

		THREAD_MAIN(GUIThread);
		virtual void 	exit();

		bool 			get_runtime_state();
		void 			set_runtime_state(bool state);

	protected:
		pthread_mutex_t 	_m_runtime;
		bool 				_runtime_state;
};

#endif

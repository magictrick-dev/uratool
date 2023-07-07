
Some core notes on the development of uratool.

- [Mass Storage Device Enumeration](https://stackoverflow.com/questions/20562263/enumerate-usb-flash-drives-programatically-using-libudev-in-linux)
    
    Searching for block devices...


* Multithreading

```C++
static MyThread::Main(void* arguments)
{
    MyThread* self = (MyThread*)arguments;
}

void Thread::
launch()
{
    this->_thread_handle = pthread_create(&this->_thread_handle, NULL,
        this->_thread_func, (void*)this);
}

// This is pretty nasty, but theoretically, it works, right?
template <class T>
ThreadingManager::create_thread()
{
    T* instance = new T(T::Main);
    this->_active_threads.push_back(instance);
}

// We create the thread which returns a MyThread class. This
// registers that thread with ThreadingManager but doesn't create
// an actual pthread until launch() is invoked.
MyThread* my_thread = ThreadingManager.create_thread<MyThread>();
my_thread->launch();

```

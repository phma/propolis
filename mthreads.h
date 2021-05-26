#ifdef __MINGW64__
#define MINGW_STDTHREAD_REDUNDANCY_WARNING
#include <../mingw-std-threads/mingw.thread.h>
#include <../mingw-std-threads/mingw.mutex.h>
#include <../mingw-std-threads/mingw.shared_mutex.h>
#else
#include <thread>
#include <mutex>
#include <shared_mutex>
#endif

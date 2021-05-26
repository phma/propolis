#ifndef THREADS_H
#define THREADS_H

#include <chrono>
#include <vector>
#include <array>
#include "mthreads.h"
#include "letters.h"

// These are used as both commands to the threads and status from the threads.
#define TH_RUN 1
#define TH_PAUSE 2
#define TH_WAIT 3
#define TH_STOP 7
#define TH_ASLEEP 256

extern std::chrono::steady_clock clk;

void startThreads(int n);
void joinThreads();
void enqueueInvLetterTask(InvLetterTask task);
InvLetterTask dequeueInvLetterTask();
void enqueueInvLetterResult(InvLetterResult result);
InvLetterResult dequeueInvLetterResult();
void sleepRead();
void sleep(int thread);
void sleepDead(int thread);
void unsleep(int thread);
double maxSleepTime();
void randomizeSleep();
void setThreadCommand(int newStatus);
int getThreadStatus();
void waitForThreads(int newStatus);
void waitForQueueEmpty();

class PropThread
{
public:
  void operator()(int thread);
};

#endif

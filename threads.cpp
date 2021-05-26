#include <queue>
#include <iostream>
#include <cmath>
#include "threads.h"
using namespace std;
namespace cr=std::chrono;

shared_mutex adjLog;
mutex actMutex;
mutex bucketMutex;
mutex startMutex;
mutex opTimeMutex;
mutex blockTaskMutex;
mutex contourMutex;

int threadCommand;
vector<thread> threads;
vector<int> threadStatus; // Bit 8 indicates whether the thread is sleeping.
vector<double> sleepTime,sleepFraction;
//mutex 

cr::steady_clock clk;
double opTime; // time for triop and edgeop, in milliseconds

double busyFraction()
{
  int i,numBusy=0;
  for (i=0;i<threadStatus.size();i++)
    if ((threadStatus[i]&256)==0)
      numBusy++;
  return (double)numBusy/i;
}

void startThreads(int n)
{
  int i,m;
  threadCommand=TH_WAIT;
  sleepTime.resize(n);
  sleepFraction.resize(n);
  opTime=0;
  for (i=0;i<n;i++)
  {
    sleepFraction[i]=0.5;
    threads.push_back(thread(PropThread(),i));
    this_thread::sleep_for(chrono::milliseconds(10));
  }
}

void joinThreads()
{
  int i;
  for (i=0;i<threads.size();i++)
    threads[i].join();
}

void sleepCommon(cr::steady_clock::time_point wakeTime,int thread)
{
  while (clk.now()<wakeTime)
  {
    if (true /*errorQueueEmpty()*/)
    {
      threadStatus[thread]|=256;
      this_thread::sleep_for((wakeTime-clk.now())*sleepFraction[thread]);
      sleepFraction[thread]*=1.25;
      if (sleepFraction[thread]>0.5)
	sleepFraction[thread]*=0.75;
      threadStatus[thread]&=255;
    }
    else
    {
      //ErrorBlockTask etask=dequeueError();
      //computeErrorBlock(etask);
      sleepFraction[thread]*=0.75;
      if (sleepFraction[thread]*sleepTime[thread]<0.001)
	sleepFraction[thread]*=1.5;
    }
  }
}

void sleep(int thread)
{
  sleepTime[thread]+=1+sleepTime[thread]/1e3;
  if (sleepTime[thread]>opTime*sleepTime.size()/2+500)
    sleepTime[thread]=opTime*sleepTime.size()/2+500;
  cr::steady_clock::time_point wakeTime=clk.now()+cr::milliseconds(lrint(sleepTime[thread]));
  sleepCommon(wakeTime,thread);
}

void unsleep(int thread)
{
  sleepTime[thread]-=1+sleepTime[thread]/1e3;
  if (sleepTime[thread]<0 || std::isnan(sleepTime[thread]))
    sleepTime[thread]=0;
}

double maxSleepTime()
{
  int i;
  double max=0;
  for (i=0;i<sleepTime.size();i++)
    if (sleepTime[i]>max)
      max=sleepTime[i];
  return max;
}

void updateOpTime(cr::nanoseconds duration)
{
  double time=duration.count()/1e6;
  opTimeMutex.lock();
  opTime*=0.999;
  if (time>opTime)
    opTime=time;
  opTimeMutex.unlock();
}

void setThreadCommand(int newStatus)
{
  threadCommand=newStatus;
  //cout<<statusNames[newStatus]<<endl;
}

int getThreadStatus()
/* Returns aaaaaaaaaabbbbbbbbbbcccccccccc where
 * aaaaaaaaaa is the status all threads should be in,
 * bbbbbbbbbb is 0 if all threads are in the same state, and
 * cccccccccc is the state the threads are in.
 * If all threads are in the commanded state, but some may be asleep and others awake,
 * getThreadStatus()&0x3ffbfeff is a multiple of 1048577.
 */
{
  int i,oneStatus,minStatus=-1,maxStatus=0;
  for (i=0;i<threadStatus.size();i++)
  {
    oneStatus=threadStatus[i];
    maxStatus|=oneStatus;
    minStatus&=oneStatus;
  }
  return (threadCommand<<20)|((minStatus^maxStatus)<<10)|(minStatus&0x3ff);
}

void waitForThreads(int newStatus)
// Waits until all threads are in the commanded status.
{
  int i,n;
  threadCommand=newStatus;
  do
  {
    for (i=n=0;i<threadStatus.size();i++)
      if ((threadStatus[i]&255)!=threadCommand)
	n++;
    this_thread::sleep_for(chrono::milliseconds(n));
  } while (n);
}

void PropThread::operator()(int thread)
{
  int e=0,t=0,d=0;
  int triResult,edgeResult;
  startMutex.lock();
  if (threadStatus.size()!=thread)
  {
    cout<<"Starting thread "<<threadStatus.size()<<", was passed "<<thread<<endl;
    thread=threadStatus.size();
  }
  threadStatus.push_back(0);
  startMutex.unlock();
  while (threadCommand!=TH_STOP)
  {
    if (threadCommand==TH_RUN)
    {
      threadStatus[thread]=TH_RUN;
      //sleep
    }
    if (threadCommand==TH_PAUSE)
    { // The job is ongoing, but has to pause to write out the files.
    }
    if (threadCommand==TH_WAIT)
    { // There is no job. The threads are waiting for a job.
    }
  }
  threadStatus[thread]=TH_STOP;
}

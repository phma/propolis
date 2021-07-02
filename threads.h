/******************************************************/
/*                                                    */
/* threads.h - multithreading                         */
/*                                                    */
/******************************************************/
/* Copyright 2019-2021 Pierre Abbat.
 * This file is part of Propolis.
 * 
 * The Propolis program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Propolis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License and Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Propolis. If not, see <http://www.gnu.org/licenses/>.
 */
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
int taskQueueSize();
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

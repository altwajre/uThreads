/*
 * kThread.h
 *
 *  Created on: Oct 27, 2014
 *      Author: Saman Barghi
 */

#ifndef KTHREAD_H_
#define KTHREAD_H_
#include <thread>
#include <mutex>

#include "../generic/basics.h"
#include "runtime/Cluster.h"
#include "runtime/uThread.h"
#include "io/IOHandler.h"


class kThread {
	friend class uThread;
	friend class Cluster;
	friend class ReadyQueue;
	friend class LibInitializer;

private:
	kThread(bool);							//This is only for the initial kThread
	std::thread *threadSelf;				//pthread related to the current thread
	uThread* mainUT;						//Each kThread has a default uThread that is used when there is no work available
    bool shouldSpin;                        //Should kThread spin before blocking

	static kThread defaultKT;				//default main thread of the application
	static kThread ioKT;                   //Dedicated IO kThread
	/* make user create the kernel thread for ths syscalls as required */
	//static kThread* syscallKT;				//syscall kernel thread for the application


	Cluster* localCluster;					//Pointer to the cluster that provides jobs for this kThread

	static __thread IntrusiveList<Thread> ktReadyQueue;	//internal readyQueue for kThread, to avoid locking and unlocking the cluster ready queue

	void run();						//The run function for the thread.
	void initialize(bool);				//Initialization function for kThread
	static inline void postSwitchFunc(uThread*, void*) __noreturn;
    void spin();

    void initialSynchronization();
    IOHandler* newIOHandler();

public:
	kThread();
	kThread(Cluster*);
	virtual ~kThread();

	uThread* currentUT;						//Pointer to the current running ut
	static __thread kThread* currentKT;
	static IOHandler* ioHandler;            //Thread local iohandler (epoll, poll, select wrapper)

	static std::atomic_uint totalNumberofKTs;

	void switchContext(uThread*,void* args = nullptr);			//Put current uThread in ready Queue and run the passed uThread
	void switchContext(void* args = nullptr);					//Put current uThread in readyQueue and pop a new uThread to run
	static void defaultRun(void*) __noreturn;


	void printThreadId();
	std::thread::native_handle_type getThreadNativeHandle();
	std::thread::id	getThreadID();

    void setShouldSpin(bool);
};

#endif /* KTHREAD_H_ */

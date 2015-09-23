/*
 * IOHandler.h
 *
 *  Created on: Aug 13, 2015
 *      Author: Saman Barghi
 */

#ifndef IOHANDLER_H_
#define IOHANDLER_H_

#include <unordered_map>
#include <vector>
#include <sys/socket.h>
#include <mutex>
#include "uThread.h"

/*
 * Include poll data
 */
class PollData{
public:
    enum states {
        IDLE,
        READY,
        WAIT,
        PARKED
    };
    std::mutex mtx;                                     //Mutex that protects this  PollData
    int fd;                                            //file descriptor
    bool newFD = true;                                 // Just opened?

    short readState = IDLE;                             //read state
    short writeSate = IDLE;                             //write state
    uThread* rut = nullptr;
    uThread* wut = nullptr;                             //read and write uThreads

    PollData( int fd) : fd(fd) {};
    ~PollData(){};

};
/*
 * This class is a virtual class to provide nonblocking I/O
 * to uThreads. select/poll/epoll or other type of nonblocking
 * I/O can have their own class that inherits from IOHandler.
 * The purpose of this class is to be used as a thread local
 * I/O handler per kThread.
 */
class IOHandler{
    virtual void _Open(int fd) = 0;         //Add current fd to the polling list, and add current uThread to IOQueue
    /*
     * Timeout can be int for epoll, timeval for select or timespec for poll.
     * Here the assumption is timeout is passed in milliseconds as an integer, similar to epoll.
     */
    virtual void _Poll(int timeout)=0;                ///
protected:
    IOHandler(){};
    void PollReady(int fd, int flag);                   //When there is notification update pollData and unblock the related ut
    /*
     * When a uThread request I/O, the device will be polled
     * and uThread should be added to IOTable.
     * This data structure maps file descriptors to uThreads, so when
     * the device is ready, this structure is used as a look up table to
     * put the waiting uThreads back on the related readyQueue.
     */
    std::unordered_multimap<int, PollData*> IOTable;
    std::mutex iomutex;
public:
   ~IOHandler(){};  //should be protected
    /* public interfaces */
   void open(int fd, int flag);
   void poll(int timeout, int flag);
   //dealing with uThreads

   //IO function for dedicated kThread
   static void defaultIOFunc(void*) __noreturn;

   //IO functions
   ssize_t recv(int sockfd, void *buf, size_t len, int flags);
   ssize_t send(int sockfd, const void *buf, size_t len, int flags);
   int accept(int   sockfd, struct sockaddr *addr, socklen_t *addrlen);
};

/* epoll wrapper */
class EpollIOHandler : public IOHandler {
private:
    static const int MAXEVENTS  = 128;                           //Maximum number of events this thread can monitor, TODO: do we want this to be modified?
    int epoll_fd = -1;
    struct epoll_event* events;

    void _Open(int fd);
    void _Poll(int timeout);
public:
    EpollIOHandler();
    virtual ~EpollIOHandler();
};


#endif /* IOHANDLER_H_ */
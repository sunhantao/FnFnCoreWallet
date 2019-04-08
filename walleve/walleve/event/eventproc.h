// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_EVENT_PROC_H
#define  WALLEVE_EVENT_PROC_H

#include "walleve/base/base.h"
#include "walleve/event/event.h"

#include <queue>
#include <mutex>
#include <condition_variable>

namespace walleve
{

class CWalleveEventQueue
{
public:
    CWalleveEventQueue() : fAbort(false) {}
    ~CWalleveEventQueue() {Reset();}
    void AddNew(CWalleveEvent* p)
    {
        {
            std::unique_lock<std::mutex> lock(mutex);
            que.push(p);
        }
        cond.notify_one();
    }
    CWalleveEvent* Fetch()
    {
        CWalleveEvent* p = NULL;
        std::unique_lock<std::mutex> lock(mutex);
        while (!fAbort && que.empty())
        {
            cond.wait(lock);
        }
        if (!fAbort && !que.empty())
        {
            p = que.front();
            que.pop();
        }
        return p;
    }
    void Reset()
    {
        std::unique_lock<std::mutex> lock(mutex);
        while(!que.empty())
        {
            que.front()->Free();
            que.pop();
        }
        fAbort = false;
    }
    void Interrupt()
    {
        {
            std::unique_lock<std::mutex> lock(mutex);
            while(!que.empty())
            {
                que.front()->Free();
                que.pop();
            }
            fAbort = true;
        }
        cond.notify_all();
    }
protected:
    std::condition_variable cond;
    std::mutex mutex;
    std::queue<CWalleveEvent*> que;
    bool fAbort;
};

class CWalleveEventProc : public IWalleveBase
{
public:
    CWalleveEventProc(const std::string& walleveOwnKeyIn, const size_t nThreadIn = 1, const bool fAffinityIn = false);
    void PostEvent(CWalleveEvent * pEvent);
protected:
    bool WalleveHandleInvoke() override;
    void WalleveHandleHalt() override;
    void EventThreadFunc();
protected:
    size_t nThreadNum;
    bool fAffinity;
    std::vector<CWalleveThread> vecThrEventQue;
    CWalleveEventQueue queEvent;
};

} // namespace walleve

#endif //WALLEVE_EVENT_PROC_H


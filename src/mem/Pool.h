/*
 * Copyright (C) 1996-2022 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */

#ifndef _MEM_POOL_H_
#define _MEM_POOL_H_

/**
 \defgroup MemPoolsAPI  Memory Management (Memory Pool Allocator)
 \ingroup Components
 *
 *\par
 *  MemPools are a pooled memory allocator running on top of malloc(). It's
 *  purpose is to reduce memory fragmentation and provide detailed statistics
 *  on memory consumption.
 *
 \par
 *  Preferably all memory allocations in Squid should be done using MemPools
 *  or one of the types built on top of it (i.e. cbdata).
 *
 \note Usually it is better to use cbdata types as these gives you additional
 *     safeguards in references and typechecking. However, for high usage pools where
 *     the cbdata functionality of cbdata is not required directly using a MemPool
 *     might be the way to go.
 */

#include "mem/Allocator.h"
#include "mem/Meter.h"
#include "util.h"

#include <list>
#if HAVE_GNUMALLOC_H
#include <gnumalloc.h>
#elif HAVE_MALLOC_H
#include <malloc.h>
#endif
#if HAVE_MEMORY_H
#include <memory.h>
#endif

#if !M_MMAP_MAX
#if USE_DLMALLOC
#define M_MMAP_MAX -4
#endif
#endif

/// \ingroup MemPoolsAPI
#define toMB(size) ( ((double) size) / ((double)(1024*1024)) )
/// \ingroup MemPoolsAPI
#define toKB(size) ( (size + 1024 - 1) / 1024 )

/// \ingroup MemPoolsAPI
#define MEM_PAGE_SIZE 4096
/// \ingroup MemPoolsAPI
#define MEM_MIN_FREE  32
/// \ingroup MemPoolsAPI
#define MEM_MAX_FREE  65535 /* unsigned short is max number of items per chunk */

class MemImplementingAllocator;

/// memory usage totals as of latest MemPools::flushMeters() event
extern Mem::PoolMeter TheMeter;

/// \ingroup MemPoolsAPI
class MemPools
{
public:
    static MemPools &GetInstance();
    MemPools();
    void flushMeters();

    /**
     * Create an allocator with given name to allocate fixed-size objects
     * of the specified size.
     */
    MemImplementingAllocator *create(const char *, size_t);

    /**
     * Sets upper limit in bytes to amount of free ram kept in pools. This is
     * not strict upper limit, but a hint. When MemPools are over this limit,
     * deallocate attempts to release memory to the system instead of pooling.
     */
    void setIdleLimit(const ssize_t newLimit) { idleLimit_ = newLimit; }
    /// \copydoc idleLimit_
    ssize_t idleLimit() const { return idleLimit_; }

    /**
     \par
     * Main cleanup handler. For MemPools to stay within upper idle limits,
     * this function needs to be called periodically, preferably at some
     * constant rate, eg. from Squid event. It looks through all pools and
     * chunks, cleans up internal states and checks for releasable chunks.
     *
     \par
     * Between the calls to this function objects are placed onto internal
     * cache instead of returning to their home chunks, mainly for speedup
     * purpose. During that time state of chunk is not known, it is not
     * known whether chunk is free or in use. This call returns all objects
     * to their chunks and restores consistency.
     *
     \par
     * Should be called relatively often, as it sorts chunks in suitable
     * order as to reduce free memory fragmentation and increase chunk
     * utilisation.
     * Suitable frequency for cleanup is in range of few tens of seconds to
     * few minutes, depending of memory activity.
     *
     * TODO: DOCS: Re-write this shorter!
     *
     \param maxage   Release all totally idle chunks that
     *               have not been referenced for maxage seconds.
     */
    void clean(time_t maxage);

    void setDefaultPoolChunking(bool const &);

    std::list<MemImplementingAllocator *> pools;
    bool defaultIsChunked = false;

private:
    /// Limits the cumulative size of allocated (but unused) memory in all pools.
    /// Initial value is 2MB until first configuration,
    /// See squid.conf memory_pools_limit directive.
    ssize_t idleLimit_ = (2 << 20);
};

/// \ingroup MemPoolsAPI
class MemImplementingAllocator : public Mem::Allocator
{
public:
    typedef Mem::PoolMeter PoolMeter; // TODO remove

    MemImplementingAllocator(char const *aLabel, size_t aSize);

    virtual PoolMeter &getMeter();
    virtual void flushMetersFull();
    virtual void flushMeters();
    virtual bool idleTrigger(int shift) const = 0;
    virtual void clean(time_t maxage) = 0;

    /* Mem::Allocator API */
    virtual PoolMeter const &getMeter() const;
    virtual void *alloc();
    virtual void freeOne(void *);
    virtual size_t objectSize() const;
    virtual int getInUseCount() = 0;

protected:
    virtual void *allocate() = 0;
    virtual void deallocate(void *, bool aggressive) = 0;
    PoolMeter meter;
public:
    size_t alloc_calls;
    size_t free_calls;
    size_t saved_calls;
    size_t obj_size;
};

/// Creates a named MemPool of elements with the given size
#define memPoolCreate MemPools::GetInstance().create

#endif /* _MEM_POOL_H_ */


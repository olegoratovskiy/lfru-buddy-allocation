#pragma once

#include <algorithm>
#include <cstddef>
#include <list>
#include <new>
#include <ostream>

template <class Key, class KeyProvider, class Allocator>
class Cache
{
public:
    template <class... AllocArgs>
    Cache(const std::size_t cache_size, AllocArgs &&... alloc_args)
        : m_max_top_size(cache_size)
        , m_max_low_size(cache_size)
        , m_alloc(std::forward<AllocArgs>(alloc_args)...)
    {
    }

    std::size_t size() const
    {
        return m_queue_low.size() + m_queue_top.size();
    }

    bool empty() const
    {
        return size() == 0;
    }

    template <class T>
    T & get(const Key & key);

    std::ostream & print(std::ostream & strm) const;

    friend std::ostream & operator<<(std::ostream & strm, const Cache & cache)
    {
        return cache.print(strm);
    }

private:
    const std::size_t m_max_top_size;
    const std::size_t m_max_low_size;
    Allocator m_alloc;
    std::list<KeyProvider *> m_queue_top;
    std::list<KeyProvider *> m_queue_low;
};

template <class Key, class KeyProvider, class Allocator>
template <class T>
inline T & Cache<Key, KeyProvider, Allocator>::get(const Key & key)
{
    auto it = std::find_if(m_queue_top.begin(), m_queue_top.end(), [&key](const KeyProvider * ptr) {
        return *ptr == key;
    });е
    if (it != m_queue_top.end()) {
        m_queue_top.splice(m_queue_top.begin(), m_queue_top, it);
        return *static_cast<T *>(m_queue_top.front());
    }
    else {
        it = std::find_if(m_queue_low.begin(), m_queue_low.end(), [&key](const KeyProvider * ptr) {
            return *ptr == key;
        });
        bool was_in_low = false;
        if (it != m_queue_low.end()) {
            was_in_low = true;
            m_queue_top.splice(m_queue_top.begin(), m_queue_low, it);
            if (m_max_top_size < m_queue_top.size()) {
                m_queue_low.push_front(m_queue_top.back());
                m_queue_top.pop_back();
            }
        }
        else {
            m_queue_low.push_front(m_alloc.template create<T>(key));
        }
        if (m_max_low_size < m_queue_low.size()) {
            m_alloc.template destroy<KeyProvider>(m_queue_low.back());
            m_queue_low.pop_back();
        }
        return *static_cast<T *>(was_in_low ? m_queue_top.front() : m_queue_low.front());
    }
}

template <class Key, class KeyProvider, class Allocator>
inline std::ostream & Cache<Key, KeyProvider, Allocator>::print(std::ostream & strm) const
{
    strm << "Priority:";
    for (const auto ptr : m_queue_top) {
        strm << " " << *ptr;
    }
    strm << "\nRegular:";
    for (const auto ptr : m_queue_low) {
        strm << " " << *ptr;
    }
    return strm << "\n";
}

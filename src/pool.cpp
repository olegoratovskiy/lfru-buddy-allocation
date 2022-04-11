#include "pool.h"

#include <cmath>

namespace {
std::size_t pow2(std::size_t n)
{
    return (1 << n);
}

std::size_t ceil_log2(std::size_t n)
{
    for (std::size_t i = 0; i < 32; ++i) {
        if (pow2(i) >= n) {
            return i;
        }
    }
    return -1;
}
} // namespace

void * PoolAllocator::allocate(const std::size_t n) // передается размер блока
{
    std::size_t pos = npos;
    for (std::size_t k = ceil_log2(n); k <= m_max_p && pos == npos; ++k) {
        pos = find_empty_place(pow2(k));
    }
    if (pos == npos) {
        throw std::bad_alloc{};
    }
    while (m_size_map[pos] / 2 >= n && m_size_map[pos] / 2 >= pow2(m_min_p)) {
        separate(pos);
    }
    m_used_map[pos] = true;
    return &m_storage[pos * pow2(m_min_p)];
}

void PoolAllocator::separate(std::size_t pos)
{
    std::size_t half = m_size_map[pos] / 2;
    m_size_map[pos] = half;
    m_size_map[pos + half / pow2(m_min_p)] = half;
}

void PoolAllocator::connect_with_right(std::size_t pos)
{
    std::size_t new_size = m_size_map[pos] * 2;
    m_size_map[pos] = new_size;
    m_size_map[pos + new_size / pow2(m_min_p + 1)] = 0;
}

void PoolAllocator::deallocate(const void * ptr)
{
    auto b_ptr = static_cast<const std::byte *>(ptr);
    const auto begin = &m_storage[0];
    std::size_t pos = (b_ptr - begin) / pow2(m_min_p);
    m_used_map[pos] = false;
    bool saw_buddy = true;
    while (saw_buddy) {
        saw_buddy = false;
        std::size_t narrowed_size = m_size_map[pos] / pow2(m_min_p);
        if (pos + narrowed_size < m_size_map.size() && !m_used_map[pos + narrowed_size] &&
            m_size_map[pos + narrowed_size] == m_size_map[pos]) {
            connect_with_right(pos);
            saw_buddy = true;
        }
        if (pos >= narrowed_size && !m_used_map[pos - narrowed_size] &&
            m_size_map[pos - narrowed_size] == m_size_map[pos]) {
            pos -= narrowed_size;
            saw_buddy = true;
        }
    }
}

std::size_t PoolAllocator::find_empty_place(const std::size_t n) const // передается размер блока
{
    for (std::size_t i = 0; i < m_size_map.size(); ++i) {
        if (!m_used_map[i] && m_size_map[i] == n) {
            return i;
        }
    }
    return npos;
}

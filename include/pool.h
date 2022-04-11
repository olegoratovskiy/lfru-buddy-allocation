#pragma once

#include <cstddef>
#include <new>
#include <vector>

class PoolAllocator
{
private:
    const unsigned m_min_p;              // максимальная степень двойки
    const unsigned m_max_p;              // минимальная степень двойки
    std::vector<std::size_t> m_size_map; // размер блока который начинается в i-ой позиции
    std::vector<bool> m_used_map;        // занят ли блок
    std::vector<std::byte> m_storage;    // храним данные какие-то

    static constexpr std::size_t npos = -1;

    std::size_t find_empty_place(std::size_t n) const; // вернет индекс
    void separate(std::size_t pos);
    void connect_with_right(std::size_t pos);

public:
    PoolAllocator(const unsigned min_p, const unsigned max_p)
        : m_min_p(min_p)
        , m_max_p(max_p)
        , m_size_map(1 << (max_p - min_p))
        , m_used_map(1 << (max_p - min_p))
        , m_storage(1 << max_p)
    {
        m_size_map[0] = (1 << max_p);
    }

    void * allocate(const std::size_t n);

    void deallocate(const void * ptr);
};

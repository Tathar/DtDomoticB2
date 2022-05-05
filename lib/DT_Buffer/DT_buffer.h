#pragma once
// #include <Arduino.h>

// #include <stdint.h>
// #include <stddef.h>
// #include <DT_ha.h>

template <typename T>
class DT_buffer
{
public:
    DT_buffer();
    ~DT_buffer();

    /**
     * Disables copy constructor
     */
    DT_buffer(const DT_buffer &) = delete;
    DT_buffer(DT_buffer &&) = delete;

    /**
     * Disables assignment operator
     */
    DT_buffer &operator=(const DT_buffer &) = delete;
    DT_buffer &operator=(DT_buffer &&) = delete;

    void push(T data);
    T shift();

    inline uint8_t capacity() const
    {
        return buffer_len;
    };

    inline uint8_t available() const
    {
        return buffer_len - count;
    };

    inline uint8_t size() const
    {
        return count;
    };

    void reserve(uint8_t Size);

    void clear();
    void clean(uint8_t Size);

private:
    // void DT_buffer::cls_debug();

    T *buffer;
    uint8_t tail;
    uint8_t head;
    uint8_t buffer_len;
    uint8_t count;
};

#include <DT_buffer.tpp>
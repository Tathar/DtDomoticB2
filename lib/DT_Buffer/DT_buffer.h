#pragma once
#include <Arduino.h>

#include <stdint.h>
#include <stddef.h>
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

    void append(T &data);
    T get();

    uint8_t inline size() const
    {
        return (buffer_len);
    };

    uint8_t avalible() const;
    uint8_t usage() const;

    void reseve(uint8_t);

    void clear();

private:
    // void DT_buffer::cls_debug();

    T *buffer;
    uint8_t tail;
    uint8_t head;
    uint8_t buffer_len;
};

#include <DT_buffer.tpp>
#include <DT_buffer.h>
#include <DT_ha.h>
#include <config.h>

template <typename T>
DT_buffer<T>::DT_buffer()
{
    buffer = nullptr;
    buffer_len = 0;
}

template <typename T>
DT_buffer<T>::~DT_buffer()
{
    if (buffer != nullptr)
    {
        free(buffer);
        buffer = nullptr;
        buffer_len = tail = head = 0;
    }
}

template <typename T>
void DT_buffer<T>::append(T &data)
{
    // debug(AT);

    if (buffer == nullptr)
    {
        // debug(AT);
        buffer_len = 1;
        buffer = (T*) malloc(sizeof(T));
        buffer[tail] = T(data);
        ++tail;
    }
    else
    {
        // debug(AT);
        reseve(1);

        if (tail == buffer_len)
        {
            tail = 0;
        }

        buffer[tail] = T(data);
        ++tail;
    }
    // debug(AT);
}

template <typename T>
T DT_buffer<T>::get()
{
    T ret;
    if (buffer != nullptr)
    {
        // debug(AT);
        // cls_debug();
        if (head == buffer_len)
        {
            // debug(AT);
            head = 0;
        }

        ret = T(buffer[head]);
        ++head;

        if (head == tail)
        {
            free(buffer);
            buffer = nullptr;
            buffer_len = head = tail = 0;
        }
        return ret;
    }
    return ret;
}

template <typename T>
void DT_buffer<T>::clear()
{
    if (buffer != nullptr)
    {
        free(buffer);
        buffer = nullptr;
        buffer_len = head = tail = 0;
    }
}

template <typename T>
void DT_buffer<T>::reseve(uint8_t Size)
{
    if (buffer == nullptr)
    {
        // debug(AT);
        buffer_len = Size;
        buffer = (T *)malloc(sizeof(T) * Size);
        // head = tail = 0;
    }
    else
    {

        // debug(AT);
        // cls_debug();
        // if (((tail - head <= 0) && (head - tail < size)) || ((tail - head > 0) && buffer_len - (tail - head) < size))
        if (avalible() < Size)
        {
            uint8_t size = Size - avalible();
            // Serial.println(head_offset);
            buffer = (T *)realloc(buffer, sizeof(T) * (buffer_len + size));
            if (tail - head <= 0) // la tete est deriere la queud
            {
                for (uint8_t num = 0; num < (buffer_len - head); ++num)
                {
                    // debug(AT);
                    uint8_t from_indx = buffer_len - 1 - num;
                    uint8_t dest_index = buffer_len + size - 1 - num;
                    memcpy((void *)&buffer[dest_index], (void *)&buffer[from_indx], sizeof(T));
                }

                head += size;
            }
            buffer_len += size;
        }
    }
}

template <typename T>
uint8_t DT_buffer<T>::avalible() const
{
    return tail - head <= 0 ? head - tail : buffer_len - (tail - head);
}

template <typename T>
uint8_t DT_buffer<T>::usage() const
{
    return buffer_len - avalible();
}

// void DT_buffer<T>::cls_debug()
// {
//     uint8_t head_offset = head - buffer;
//     uint8_t tail_offset = tail - buffer;
//     Serial.print(F("head = "));
//     Serial.println(head_offset);

//     Serial.print(F("tail = "));
//     Serial.println(tail_offset);

//     Serial.print(F("free size = "));
//     if (tail - head <= 0)
//         Serial.println(head - tail);
//     else
//         Serial.println(buffer_len - (tail - head));
// }

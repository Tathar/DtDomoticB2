#include "DT_buffer.h"
// #include <cstdint>
// #include <stdlib.h>
// #include <cstring>

template <typename T>
DT_buffer<T>::DT_buffer()
{
    buffer = nullptr;
    buffer_len = count = tail = head = 0;
}

template <typename T>
DT_buffer<T>::~DT_buffer()
{
    if (buffer != nullptr)
    {
        while (available() != 0)
        {
            T data = shift();
        }

        free(buffer);
        buffer = nullptr;
        buffer_len = count = tail = head = 0;
    }
}

template <typename T>
void DT_buffer<T>::push(T data)
{
    // debug(AT);

    // Serial.print(F("buffer append "));
    // MQTT_data_debug(data);

    // debug(AT);
    reserve(1);

    if (available() == 0)
        return;

    if (tail >= buffer_len)
    {
        tail = 0;
    }

    buffer[tail] = T(data);
    // MQTT_data_debug(buffer[tail]);
    ++tail;
    ++count;

    // Serial.print(F("buffer append "));
    // // MQTT_data_debug(ret);
    // Serial.print(F("buffer_len = "));
    // Serial.println(buffer_len);
    // Serial.print(F(" available = "));
    // Serial.println(available());
    // Serial.print(F(" available = "));
    // Serial.println(available());
    // Serial.print(F("head = "));
    // Serial.println(head);
    // Serial.print(F("tail = "));
    // Serial.println(tail);
    // debug(AT);
}

template <typename T>
T DT_buffer<T>::shift()
{
    T ret;
    if (buffer != nullptr && count >= 0)
    {
        // debug(AT);
        // cls_debug();
        if (head >= buffer_len)
        {
            // debug(AT);
            head = 0;
        }

        ret = T(buffer[head]);

        ++head;
        --count;
        if (count == 0)
        {
            head = tail = 0;
        }

        // Serial.print(F("buffer get "));
        // // MQTT_data_debug(ret);
        // Serial.print(F("buffer_len = "));
        // Serial.println(buffer_len);
        // Serial.print(F(" available = "));
        // Serial.println(available());
        // Serial.print(F(" available = "));
        // Serial.println(available());
        // Serial.print(F("head = "));
        // Serial.println(head);
        // Serial.print(F("tail = "));
        // Serial.println(tail);
        return ret;
    }
    // debug(AT);
    return ret;
}

template <typename T>
void DT_buffer<T>::clear()
{
    if (buffer != nullptr)
    {
        // while (available() != 0)
        // {
        //     T data = get();
        // }
        free(buffer);
        buffer = nullptr;
        buffer_len = count = head = tail = 0;
    }
}

template <typename T>
void DT_buffer<T>::clean(uint8_t Size)
{
    if (buffer != nullptr)
    {
        if (available() > Size)
        {
            uint8_t retract = available() - Size;
            // Serial.println(head_offset);

            // Serial.println(F("realloc"));
            if (head == buffer_len) // la tete est deriere la queud
            {
                head = 0;
            }
            else if (head > tail) // la tete est deriere la queud
            {
                for (uint8_t num = 0; num < (buffer_len - head); ++num)
                {
                    // debug(AT);
                    uint8_t from_indx = head + num;
                    uint8_t dest_index = (head - retract) + num;
                    memcpy((void *)&buffer[dest_index], (void *)&buffer[from_indx], sizeof(T));
                }

                head -= retract;
            }
            // else if (tail < buffer_len - retract)
            // {
            //     tail -= retract;
            // }
            else if (tail >= buffer_len - retract) // la tete est devent la queud
            {
                uint8_t offset = retract - (buffer_len - tail);
                for (uint8_t num = 0; num < (tail - head); ++num)
                {
                    // debug(AT);
                    uint8_t from_indx = head + num;
                    uint8_t dest_index = (head - offset ) + num;
                    memcpy((void *)&buffer[dest_index], (void *)&buffer[from_indx], sizeof(T));
                }
                head -= offset;
                tail -= offset;
            }
            buffer_len -= retract;

            buffer = (T *)realloc(buffer, sizeof(T) * buffer_len);

        }
    }
}

template <typename T>
void DT_buffer<T>::reserve(uint8_t Size)
{
    // Serial.print(F("buffer reserve "));
    // Serial.println(Size);

    if (buffer == nullptr)
    {
        // Serial.println(F("nullptr"));
        buffer = (T *)malloc(sizeof(T) * Size);
        if (buffer != nullptr)
        {
            buffer_len = Size;
            count = head = tail = 0;
        }
    }
    else
    {
        // cls_debug();
        // Serial.print(F("available =  "));
        // Serial.println(available());
        if (available() < Size)
        {
            uint8_t extend = Size - available();
            // Serial.println(head_offset);

            // Serial.println(F("realloc"));
            buffer = (T *)realloc(buffer, sizeof(T) * (buffer_len + extend));

            if (head >= tail && count != 0) // la tete est deriere la queud
            {
                // Serial.println(F("move head"));
                for (uint8_t num = 0; num < (buffer_len - head); ++num)
                {
                    // debug(AT);
                    uint8_t from_indx = buffer_len - 1 - num;
                    uint8_t dest_index = buffer_len + extend - 1 - num;
                    memcpy((void *)&buffer[dest_index], (void *)&buffer[from_indx], sizeof(T));
                }

                head += extend;
            }
            buffer_len += extend;
        }
    }

    // Serial.print(F("buffer_len = "));
    // Serial.println(buffer_len);
    // Serial.print(F("head = "));
    // Serial.println(head);
    // Serial.print(F("tail = "));
    // Serial.println(tail);
}

// template <typename T>
// void DT_buffer<T>::cls_debug()
// {
//     printf("head = %i\n", head);
//     printf("tail = %i\n", tail);
//     printf("size = %i\n", size());
//     printf("avalible = %i\n", available());
//     printf("capacity = %i\n", capacity());
// }

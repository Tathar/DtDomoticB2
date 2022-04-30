#include <DT_buffer.h>
#include <DT_ha.h>
#include <config.h>

template <typename T>
DT_buffer<T>::DT_buffer()
{
    buffer = nullptr;
    buffer_len = tail = head = 0;
}

template <typename T>
DT_buffer<T>::~DT_buffer()
{
    if (buffer != nullptr)
    {
        while (usage() != 0)
        {
            T data = get();
        }

        free(buffer);
        buffer = nullptr;
        buffer_len = tail = head = 0;
    }
}

template <typename T>
void DT_buffer<T>::append(T &data)
{
    // debug(AT);

    // Serial.print(F("buffer append "));
    // MQTT_data_debug(data);

    // debug(AT);
    reseve(1);

    if (avalible() == 0)
        return;

    if (tail == buffer_len)
    {
        tail = 0;
    }

    buffer[tail] = T(data);
    // MQTT_data_debug(buffer[tail]);
    ++tail;
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

        // Serial.print(F("buffer get "));
        // MQTT_data_debug(ret);
        // Serial.print(F("buffer_len = "));
        // Serial.println(buffer_len);
        // Serial.print(F("head = "));
        // Serial.println(head);
        // Serial.print(F("tail = "));
        // Serial.println(tail);

        ++head;
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
        while (usage() != 0)
        {
            T data = get();
        }
        free(buffer);
        buffer = nullptr;
        buffer_len = head = tail = 0;
    }
}

template <typename T>
void DT_buffer<T>::clean()
{
    if (buffer != nullptr)
    {
        if (head == tail)
        {
            free(buffer);
            buffer = nullptr;
            buffer_len = head = tail = 0;
        }
        else
        {
            if (avalible() != 0)
            {
                uint8_t new_size = usage();
                // Serial.println(head_offset);

                // Serial.println(F("realloc"));
                if (tail - head <= 0) // la tete est deriere la queud
                {
                    for (uint8_t num = 0; num < (buffer_len - head); ++num)
                    {
                        // debug(AT);
                        uint8_t from_indx = head + num;
                        uint8_t dest_index = tail + num;
                        memcpy((void *)&buffer[dest_index], (void *)&buffer[from_indx], sizeof(T));
                    }

                    head = tail;
                }
                else // la tete est devent la queud
                {
                    for (uint8_t num = 0; num < (tail - head); ++num)
                    {
                        // debug(AT);
                        uint8_t from_indx = head + num;
                        uint8_t dest_index = num;
                        memcpy((void *)&buffer[dest_index], (void *)&buffer[from_indx], sizeof(T));
                    }
                    head = 0;
                    tail = new_size;
                }
                buffer_len = new_size;

                buffer = (T *)realloc(buffer, sizeof(T) * new_size);
            }
        }
        free(buffer);
        buffer = nullptr;
        buffer_len = head = tail = 0;
    }
}

template <typename T>
void DT_buffer<T>::reseve(uint8_t Size)
{
    // Serial.print(F("buffer reserve "));
    // Serial.println(Size);

    if (buffer == nullptr)
    {
        // Serial.println(F("nullptr"));
        // debug(AT);
        buffer = (T *)malloc(sizeof(T) * Size);
        if (buffer != nullptr)
        {
            buffer_len = Size;
            head = tail = 0;
        }
    }
    else
    {
        // debug(AT);
        // cls_debug();
        // Serial.print(F("avalible =  "));
        // Serial.println(avalible());
        if (avalible() < Size)
        {
            uint8_t size = Size - avalible();
            // Serial.println(head_offset);

            // Serial.println(F("realloc"));
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

    // Serial.print(F("buffer_len = "));
    // Serial.println(buffer_len);
    // Serial.print(F("head = "));
    // Serial.println(head);
    // Serial.print(F("tail = "));
    // Serial.println(tail);
}

template <typename T>
uint8_t DT_buffer<T>::avalible() const
{
    return tail - head < 0 ? head - tail : buffer_len - (tail - head);
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

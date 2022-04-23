#include <DT_buffer.h>
#include <DT_ha.h>
#include <config.h>

DT_buffer::DT_buffer()
{
    buffer = nullptr;
    buffer_len = 0;
}

DT_buffer::~DT_buffer()
{
    if (buffer != nullptr)
    {
        free(buffer);
        buffer = nullptr;
        buffer_len = 0;
    }
}

void DT_buffer::append(MQTT_data &data)
{
    // debug(AT);

    if (buffer == nullptr)
    {
        // debug(AT);
        buffer_len = 1;
        buffer = head = tail = (MQTT_data *)malloc(sizeof(MQTT_data));
        *tail = MQTT_data(data);
        ++tail;
    }
    else
    {
        // debug(AT);
        reseve(1);

        if (tail == buffer + buffer_len)
        {
            tail = buffer;
        }

        *tail = MQTT_data(data);
        ++tail;
    }
    // debug(AT);
}

MQTT_data DT_buffer::get()
{
    MQTT_data ret;
    if (buffer != nullptr)
    {
        // debug(AT);
        // cls_debug();
        if (head == buffer + buffer_len)
        {
            debug(AT);
            head = buffer;
        }

        ret = MQTT_data(*head);
        ++head;

        if (head == tail)
        {
            free(buffer);
            buffer = head = tail = nullptr;
            buffer_len = 0;
        }
        return (ret);
    }
    return ret;
}

void DT_buffer::clear()
{
    if (buffer != nullptr)
    {
        free(buffer);
        buffer = head = tail = nullptr;
        buffer_len = 0;
    }
}

void DT_buffer::reseve(uint8_t Size)
{
    if (buffer == nullptr)
    {
        // debug(AT);
        buffer_len = 1;
        buffer = head = tail = (MQTT_data *)malloc(sizeof(MQTT_data) * Size);
    }
    else
    {

        // debug(AT);
        // cls_debug();
        // if (((tail - head <= 0) && (head - tail < size)) || ((tail - head > 0) && buffer_len - (tail - head) < size))
        if (avalible() < Size)
        {
            uint8_t size = Size - avalible();
            uint8_t head_offset = head - buffer;
            uint8_t tail_offset = tail - buffer;
            // Serial.println(head_offset);
            buffer = head = tail = (MQTT_data *)realloc(buffer, sizeof(MQTT_data) * (buffer_len + size));
            head += head_offset;
            tail += tail_offset;
            if (tail - head <= 0) // la tete est deriere la queud
            {
                for (uint8_t num = 0; num < (buffer_len - head_offset); ++num)
                {
                    debug(AT);
                    uint8_t from_indx = buffer_len - 1 - num;
                    uint8_t dest_index = buffer_len + size - 1 - num;
                    memcpy((void *) &buffer[dest_index], (void *) &buffer[from_indx], sizeof(MQTT_data));
                }

                head += size;
            }
            buffer_len += size;
        }
    }
}

uint8_t DT_buffer::avalible() const
{
    return tail - head <= 0 ? head - tail : buffer_len - (tail - head);
}


uint8_t DT_buffer::usage() const
{
    return buffer_len - avalible();
}

// void DT_buffer::cls_debug()
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

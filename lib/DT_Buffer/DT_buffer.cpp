#include <DT_buffer.h>
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

void DT_buffer::append(MQTT_BUFF &data)
{
    // debug(AT);

    if (buffer == nullptr)
    {
        // debug(AT);
        buffer_len = 1;
        buffer = head = tail = (MQTT_BUFF *)malloc(sizeof(MQTT_BUFF));
        *tail = MQTT_BUFF(data);
        ++tail;
    }
    else
    {
        // debug(AT);
        if (tail == buffer + buffer_len)
        {
            uint8_t head_offset = head - buffer;
            // Serial.println(head_offset);
            buffer = head = tail = (MQTT_BUFF *)realloc(buffer, sizeof(MQTT_BUFF) * ++buffer_len);
            head += head_offset;
            tail += buffer_len - 1;
        }
        *tail = MQTT_BUFF(data);
        ++tail;
    }
    // debug(AT);
}

MQTT_BUFF DT_buffer::get()
{
    MQTT_BUFF ret;
    if (buffer == nullptr)
    {
        // debug(AT);
        return ret;
    }
    else
    {
        ret = *head;
        if (++head == tail)
        {
            free(buffer);
            buffer = head = tail = nullptr;
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

void DT_buffer::reseve(uint8_t size)
{
    if (buffer == nullptr)
    {
        // debug(AT);
        buffer_len = 1;
        buffer = head = tail = (MQTT_BUFF *)malloc(sizeof(MQTT_BUFF) * size);
    }
    else
    {
        // debug(AT);
        buffer_len += size;
        uint8_t head_offset = head - buffer;
        uint8_t tail_offset = tail - buffer;
        // Serial.println(head_offset);
        buffer = head = tail = (MQTT_BUFF *)realloc(buffer, sizeof(MQTT_BUFF) * buffer_len);
        head += head_offset;
        tail += tail_offset;
    }
}
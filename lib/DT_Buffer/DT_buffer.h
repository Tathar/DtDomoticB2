#pragma once
#include <Arduino.h>

#include <DT_ha.h>

class DT_buffer
{
    public:
    DT_buffer();
    ~DT_buffer();



	/**
	 * Disables copy constructor
	 */
	DT_buffer(const DT_buffer&) = delete;
	DT_buffer(DT_buffer&&) = delete;

	/**
	 * Disables assignment operator
	 */
	DT_buffer& operator=(const DT_buffer&) = delete;
	DT_buffer& operator=(DT_buffer&&) = delete;


    void append(MQTT_BUFF &data);
    MQTT_BUFF get();

    uint16_t size() const
    {
        return(buffer_len);
    };


    void reseve(uint8_t);

    void clear();

    private:

    MQTT_BUFF* buffer;
    MQTT_BUFF* tail;
    MQTT_BUFF* head;
    uint16_t buffer_len;
};
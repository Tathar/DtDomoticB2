
void DT_cover_init();
void DT_cover_loop();

void DT_cover_set(uint8_t num, uint8_t percent);
void DT_cover_stop(uint8_t num);
uint8_t DT_cover_get(uint8_t num);

enum cover_state
{
    cover_stopped,
    cover_closed,
    cover_open,
    cover_opening,
    cover_closing,
};

void DT_cover_set_callback(void (*callback)(const uint8_t num, const uint8_t percent, const cover_state state));

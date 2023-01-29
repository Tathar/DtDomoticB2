

enum cover_state
{
    cover_stopped,
    cover_closed,
    cover_open,
    cover_opening = 8,
    cover_closing,
};

void DT_cover_init();
void DT_cover_loop();

void DT_cover_set(uint8_t num, uint8_t percent);
void DT_cover_stop(uint8_t num);
void DT_cover_start_register(uint8_t num, bool up);
void DT_cover_stop_register(uint8_t num);
void DT_cover_backup_pos(uint8_t num);
void DT_cover_restore_pos(uint8_t num);
uint8_t DT_cover_get(uint8_t num);
cover_state DT_cover_get_state(uint8_t num);

void DT_cover_set_callback(void (*callback)(const uint8_t num, const uint8_t percent, const cover_state state));

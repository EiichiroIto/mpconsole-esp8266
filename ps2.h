extern int ps2_ClockMask;

void ps2_init(int clockPin, int dataPin);
void ps2_callback();
uint8_t ps2_get_scan_code();
bool ps2_available();
char ps2_read();
char ps2_peek();
void ps2_clear();

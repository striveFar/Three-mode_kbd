#include "../../soc/soc.h"
#include "../common/timer.h"

void timer_init(void) {
    //1ms
    /* SysTick_Config(FREQ_SYS/1000); */
}
void timer_clear(void) {
    systick_millis = 0;
}
uint16_t timer_read(void) {
    return (millis() & 0xFFFF);
}
uint32_t timer_read32(void) {
    return ((uint32_t) millis());
}
uint16_t timer_elapsed(uint16_t last) {
    return TIMER_DIFF_16(timer_read(), last);
}

uint32_t timer_elapsed32(uint32_t last) {
    return TIMER_DIFF_32(timer_read32(), last);
}


#ifndef SPEAKER_H
#define SPEAKER_H

#include <lib/stdint.h>

void speaker_init(void);
void speaker_beep(uint32_t frequency, uint32_t milliseconds);
void speaker_stop(void);

#endif /* SPEAKER_H */

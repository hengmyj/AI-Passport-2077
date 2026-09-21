/* Adapted from LEO Radio, MIT; see assets/radio/LICENSE.txt. */
#include "radio_logic.h"
#include <cstddef>
constexpr uint16_t kBandLowDecihz = 870;
constexpr uint16_t kBandHighDecihz = 1080;

bool in_band(long decihz) {
    return decihz >= kBandLowDecihz && decihz <= kBandHighDecihz;
}

bool is_digit(char value) { return value >= '0' && value <= '9'; }

// Reads "94.7" or "1017" style digits at `text` and returns tenths of a MHz.
// Digit runs without a decimal point are ambiguous: "971" is 97.1 MHz while
// "101" is 101.0 MHz. Both readings are tried and the one inside the broadcast
// band wins, checking the tenths reading first because it is far more common.
long read_decihz(const char *text, std::size_t *consumed) {
    std::size_t whole = 0;
    while (is_digit(text[whole])) ++whole;
    if (whole < 2 || whole > 4) return 0;

    long value = 0;
    for (std::size_t i = 0; i < whole; ++i) value = value * 10 + (text[i] - '0');

    if (text[whole] == '.' && is_digit(text[whole + 1])) {
        if (whole > 3 || is_digit(text[whole + 2])) return 0;
        *consumed = whole + 2;
        return value * 10 + (text[whole + 1] - '0');
    }

    *consumed = whole;
    if (whole == 2) return value * 10;      // "94" is 94.0 MHz
    if (in_band(value)) return value;       // "971" is 97.1 MHz
    if (in_band(value * 10)) return value * 10;  // "101" is 101.0 MHz
    return value;
}

uint16_t radio_parse_frequency(const char *name) {
    if (!name) return 0;

    for (const char *p = name; p[0] && p[1]; ++p) {
        if ((p[0] != 'F' && p[0] != 'f') || (p[1] != 'M' && p[1] != 'm')) continue;
        const char *digits = p + 2;
        while (*digits == ' ') ++digits;
        std::size_t consumed = 0;
        const long value = read_decihz(digits, &consumed);
        if (in_band(value)) return static_cast<uint16_t>(value);
    }

    for (const char *p = name; *p; ++p) {
        if (!is_digit(*p)) continue;
        const bool starts_run = p == name || (!is_digit(p[-1]) && p[-1] != '.');
        if (!starts_run) continue;
        std::size_t consumed = 0;
        const long value = read_decihz(p, &consumed);
        if (consumed && in_band(value)) return static_cast<uint16_t>(value);
        while (is_digit(p[1])) ++p;
    }
    return 0;
}


unsigned radio_dial_position(uint16_t frequency,unsigned index,unsigned count){
    if(frequency>=870&&frequency<=1080){if(frequency<880)return 0;return (frequency-880u)*1000u/200u;}
    if(count<2)return 500;
    if(index>=count)index=count-1;
    return index*1000u/(count-1);
}
size_t radio_downmix(int16_t *pcm,size_t bytes,unsigned channels){
    if(!pcm||channels<1||channels>2||bytes%(channels*sizeof(int16_t)))return 0;
    if(channels==1)return bytes;
    const size_t frames=bytes/4;
    for(size_t i=0;i<frames;i++)pcm[i]=static_cast<int16_t>((static_cast<int32_t>(pcm[i*2])+pcm[i*2+1])/2);
    return frames*2;
}
bool radio_timer_expired(int64_t deadline,int64_t now){return deadline>0&&now>=deadline;}

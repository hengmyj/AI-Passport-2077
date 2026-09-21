#include "radio_directory.h"
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    RadioDirectory directory;
    static_assert(sizeof(RadioDirectory) < 4400, "Keep the catalog in the original RAM budget");
    directory.replace(nullptr, 0);
    assert(directory.count() == RADIO_PRESET_COUNT && RADIO_PRESET_COUNT == 57);
    for (std::size_t i = 0; i < directory.count(); ++i) {
        assert(directory.at(i).name[0] && directory.at(i).url[0]);
        for (std::size_t j = 0; j < i; ++j) assert(std::strcmp(directory.at(i).url, directory.at(j).url));
    }
    assert(&directory.at(directory.count()) == &directory.at(0));
    RadioStation local[11] = {};
    local[0] = RADIO_PRESETS[0];
    std::strcpy(local[0].url, "https://lhttp.qtfm.cn/live/15318317/64k.mp3");
    local[1] = local[0];
    std::strcpy(local[2].url, "http://example.com/unique.mp3");
    directory.replace(local, 3);
    assert(directory.count() == RADIO_PRESET_COUNT + 1);
    assert(std::strcmp(directory.at(0).url, local[0].url) == 0);
    assert(std::strcmp(directory.at(1).url, local[2].url) == 0);
    // A lookalike hostname must not hide the legitimate curated stream.
    std::strcpy(local[0].url, "http://lhttp.qingting.fm.evil/live/15318317/64k.mp3");
    directory.replace(local, 1);
    assert(directory.count() == RADIO_PRESET_COUNT + 1);
    for (unsigned i = 0; i < 11; ++i) std::snprintf(local[i].url, sizeof(local[i].url), "http://example.com/%u.mp3", i);
    directory.replace(local, 11);
    assert(directory.count() == RADIO_PRESET_COUNT + 10);
    assert(directory.at(directory.count()-1).name[0]);
    directory.replace(nullptr, 0);
    assert(directory.count() == RADIO_PRESET_COUNT);
    std::printf("Radio directory: merge, aliases, duplicates, capacity, wrap and reset PASS (%zu bytes RAM)\n", sizeof(directory));
}

#include "radio_directory.h"
#include <algorithm>
#include <cstring>

namespace {
// Public Qingting aliases can identify the same stream with different schemes.
const char *qingting_id(const char *url) {
    const char *host = std::strstr(url, "://");
    if (!host) return nullptr;
    host += 3;
    for (const char *prefix : {"lhttp.qingting.fm/live/", "lhttp.qtfm.cn/live/"}) {
        if (std::strncmp(host, prefix, std::strlen(prefix)) == 0) return host + std::strlen(prefix);
    }
    return nullptr;
}
bool same_stream(const char *a, const char *b) {
    if (std::strcmp(a, b) == 0) return true;
    const char *x = qingting_id(a), *y = qingting_id(b);
    if (!x || !y) return false;
    const std::size_t nx = std::strspn(x, "0123456789"), ny = std::strspn(y, "0123456789");
    return nx && nx == ny && x[nx] == '/' && y[ny] == '/' && std::strncmp(x, y, nx) == 0;
}
}

void RadioDirectory::replace(const RadioStation *local, std::size_t count) {
    local_count_ = preset_count_ = 0;
    if (local) for (std::size_t i = 0; i < std::min(count, kLocalCapacity); ++i) {
        if (!local[i].url[0]) continue;
        bool duplicate = false;
        for (std::size_t j = 0; j < local_count_; ++j) duplicate |= same_stream(local[i].url, local_[j].url);
        if (!duplicate) local_[local_count_++] = local[i];
    }
    for (std::size_t i = 0; i < std::min(RADIO_PRESET_COUNT, kPresetCapacity); ++i) {
        bool duplicate = false;
        for (std::size_t j = 0; j < local_count_; ++j) duplicate |= same_stream(RADIO_PRESETS[i].url, local_[j].url);
        if (!duplicate) presets_[preset_count_++] = static_cast<uint8_t>(i);
    }
}

const RadioStation &RadioDirectory::at(std::size_t index) const {
    static const RadioStation empty = {};
    if (!count()) return empty;
    index %= count();
    return index < local_count_ ? local_[index] : RADIO_PRESETS[presets_[index - local_count_]];
}

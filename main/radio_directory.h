#pragma once
#include "radio_presets.h"

class RadioDirectory {
public:
    static constexpr std::size_t kLocalCapacity = 10;
    static constexpr std::size_t kPresetCapacity = 64;
    void replace(const RadioStation *local, std::size_t count);
    std::size_t count() const { return local_count_ + preset_count_; }
    const RadioStation &at(std::size_t index) const;
private:
    RadioStation local_[kLocalCapacity] = {};
    uint8_t presets_[kPresetCapacity] = {};
    std::size_t local_count_ = 0, preset_count_ = 0;
};

// components/bsp/include/bsp_audio.h
// ES8311 音频 codec:I2C 走控制口(复用 bsp_i2c 的共享总线),I2S 走全双工数据口。
#pragma once

#include "esp_err.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// 初始化 codec 与 I2S。内部会调 bsp_i2c_init()(幂等),无需外部先调。成功调用可重复；
// 失败会释放本次已创建的 codec 接口和 I2S channel，修正故障后可重试。
esp_err_t bsp_audio_init(void);

// 设置采样格式。同格式重复调用是廉价的(直接复用已打开的 codec)。
//
// ⚠ 这里有个必须绕开的坑:esp_codec_dev_open() 在 codec【已打开】时会直接返回 OK 且
//   【不重新配置采样率】。若不先 close,16kHz 播完再播 8kHz 会以 16k 时钟送出 ——
//   音调和速度都快一倍。格式变化时清理旧 codec 对象后重新创建并 open。
esp_err_t bsp_audio_set_format(uint32_t hz, uint8_t bits, uint8_t ch);

// 进入低功耗前强制执行 ES8311 完整 suspend 寄存器序列，回读关键寄存器
// （REG0E 只比较 bit6:0，其余寄存器比较全部位），
// 失败时重试一次，并显式停止 I2S TX/RX。该路径不依赖 codec-dev 是否打开过，
// 因此开机后从未播放也能正确暂停。调用前必须停止所有 PCM 读写；函数幂等，
// 音频未初始化时视为无需处理并返回成功。失败后再次调用会重新校验；
// 成功休眠后删除 codec 对象，保留 I2C/I2S，唤醒重新创建。
// 所有生命周期操作由独占音频任务串行调用；状态查询可跨任务。
esp_err_t bsp_audio_sleep(void);

// deep sleep 专用：确保 I2S TX/RX 已停止，再将 MCLK/BCLK/WS/DOUT/DIN
// 设为无上下拉的高阻输入。调用后 I2S 不能在本次运行中恢复，必须立即
// 进入 deep sleep 或重启；不得用于 light sleep。
esp_err_t bsp_audio_prepare_deep_sleep(void);

// 从 light sleep 返回后恢复 ES8311 和休眠前的采样格式。函数幂等；deep sleep
// 唤醒会重启应用，应由 bsp_audio_init() 按正常启动流程重新初始化。
esp_err_t bsp_audio_wake(void);
/* True also before initialization, when no codec/I2S stream is active. */
bool bsp_audio_is_sleeping(void);
/* Also true after partial open/suspend failure; not a low-power guarantee. */
bool bsp_audio_needs_wake(void);

// 播放 / 录音。bytes 为字节数(16bit 单声道时 = 采样数 x 2)。
esp_err_t bsp_audio_write(const void *pcm, size_t bytes);
esp_err_t bsp_audio_read(void *pcm, size_t bytes);

// 输出音量 0..100(%)。
void bsp_audio_set_volume(uint8_t percent);
/* Current shared codec output volume, or 40 before audio initialization.
 * Call from an audio worker, not a button callback or under the LVGL lock. */
uint8_t bsp_audio_get_volume(void);

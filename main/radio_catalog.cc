#include "radio_catalog.h"
#include "radio_logic.h"
#include "esp_timer.h"
#include "radio_http.h"
#include "radio_json_stream.h"
#include <memory>
#include <atomic>

#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

std::atomic<bool> cancelled{false};
constexpr char kTag[] = "radio_catalog";
constexpr char kLocationUrl[] =
    "http://ip-api.com/json/?fields=status,countryCode,regionName,city";
constexpr char kBackupLocationUrl[] =
    "https://ipwho.is/?fields=success,country_code,region,city";
constexpr char kDirectoryBase[] =
    "http://de1.api.radio-browser.info/json/stations/search";

struct CityTranslation {
    const char *english;
    const char *chinese;
};

// ip-api returns English city names. Radio Browser's Chinese entries are much
// easier to find by their Chinese city name, so map the most common cities and
// fall back to a province/state query for every other place.
constexpr CityTranslation kChineseCities[] = {
    {"Beijing", "北京"},       {"Shanghai", "上海"},
    {"Tianjin", "天津"},       {"Chongqing", "重庆"},
    {"Guangzhou", "广州"},     {"Shenzhen", "深圳"},
    {"Dongguan", "东莞"},      {"Foshan", "佛山"},
    {"Zhuhai", "珠海"},        {"Huizhou", "惠州"},
    {"Zhongshan", "中山"},     {"Shantou", "汕头"},
    {"Hangzhou", "杭州"},      {"Ningbo", "宁波"},
    {"Wenzhou", "温州"},       {"Jiaxing", "嘉兴"},
    {"Shaoxing", "绍兴"},      {"Jinhua", "金华"},
    {"Nanjing", "南京"},       {"Suzhou", "苏州"},
    {"Wuxi", "无锡"},          {"Changzhou", "常州"},
    {"Nantong", "南通"},       {"Xuzhou", "徐州"},
    {"Chengdu", "成都"},       {"Mianyang", "绵阳"},
    {"Wuhan", "武汉"},         {"Yichang", "宜昌"},
    {"Changsha", "长沙"},      {"Zhengzhou", "郑州"},
    {"Luoyang", "洛阳"},       {"Jinan", "济南"},
    {"Qingdao", "青岛"},       {"Yantai", "烟台"},
    {"Shenyang", "沈阳"},      {"Dalian", "大连"},
    {"Harbin", "哈尔滨"},      {"Changchun", "长春"},
    {"Shijiazhuang", "石家庄"}, {"Taiyuan", "太原"},
    {"Xi'an", "西安"},         {"Xian", "西安"},
    {"Kunming", "昆明"},       {"Guiyang", "贵阳"},
    {"Nanning", "南宁"},       {"Haikou", "海口"},
    {"Fuzhou", "福州"},        {"Xiamen", "厦门"},
    {"Nanchang", "南昌"},      {"Hefei", "合肥"},
    {"Lanzhou", "兰州"},       {"Urumqi", "乌鲁木齐"},
    {"Hohhot", "呼和浩特"},    {"Yinchuan", "银川"},
    {"Xining", "西宁"},        {"Lhasa", "拉萨"},
};

bool starts_with(const char *text, const char *prefix) {
    return text && prefix && std::strncmp(text, prefix, std::strlen(prefix)) == 0;
}

bool contains(const char *text, const char *needle) {
    return text && needle && std::strstr(text, needle) != nullptr;
}

bool http_get(const char *url, char *response, std::size_t capacity,
              std::size_t *response_size) {
    if (cancelled.load() || !url || !response || capacity < 2) return false;
    response[0] = '\0';

    esp_http_client_config_t config = {};
    config.url = url;
    config.timeout_ms = 2500;
    config.buffer_size = 2048;
    config.buffer_size_tx = 512;
    config.user_agent = "AI-Passport-Radio/1.1.1";
    config.keep_alive_enable = false;
    config.disable_auto_redirect = false;
    config.max_redirection_count = 3;
    config.crt_bundle_attach = esp_crt_bundle_attach;
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) return false;

    esp_http_client_set_header(client, "Accept", "application/json");
    esp_http_client_set_header(client, "Accept-Encoding", "identity");
    bool success = false;
    std::size_t used = 0;
    const int64_t deadline=esp_timer_get_time()+10000000;
    do {
        if (!radio_http_open(client,3) || cancelled.load()) break;
        const int status = esp_http_client_get_status_code(client);
        if (status < 200 || status >= 300) {
            ESP_LOGW(kTag, "GET %s returned HTTP %d", url, status);
            break;
        }

        while (used + 1 < capacity && !cancelled.load() && esp_timer_get_time()<deadline) {
            const int received = esp_http_client_read(
                client, response + used, static_cast<int>(capacity - used - 1));
            if (received < 0) {
                used = 0;
                break;
            }
            if (received == 0) {
                success = used > 0;
                break;
            }
            used += static_cast<std::size_t>(received);
        }
        if (used + 1 >= capacity) {
            ESP_LOGW(kTag, "Response from %s exceeded %u bytes", url,
                     static_cast<unsigned>(capacity));
            success = false;
        }
    } while (false);

    if(cancelled.load() || esp_timer_get_time()>=deadline)success=false;
    response[used] = '\0';
    if (response_size) *response_size = used;
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return success;
}

const char *json_string(cJSON *object, const char *name) {
    cJSON *item = cJSON_GetObjectItemCaseSensitive(object, name);
    return cJSON_IsString(item) && item->valuestring ? item->valuestring : "";
}

void copy_text(char *target, std::size_t capacity, const char *source) {
    if (!target || capacity == 0) return;
    std::snprintf(target, capacity, "%s", source ? source : "");
}

bool locate(RadioLocation *location) {
    char *response = static_cast<char *>(std::malloc(2048));
    if (!response) return false;
    bool success = false;
    if (http_get(kLocationUrl, response, 2048, nullptr)) {
        cJSON *root = cJSON_Parse(response);
        if (root) {
            const char *status = json_string(root, "status");
            const char *country = json_string(root, "countryCode");
            const char *region = json_string(root, "regionName");
            const char *city = json_string(root, "city");
            if (std::strcmp(status, "success") == 0 && country[0] &&
                (city[0] || region[0])) {
                copy_text(location->country_code, sizeof(location->country_code), country);
                copy_text(location->region, sizeof(location->region), region);
                copy_text(location->city, sizeof(location->city), city);
                success = true;
            }
            cJSON_Delete(root);
        }
    }
    if (!success && http_get(kBackupLocationUrl, response, 2048, nullptr)) {
        cJSON *root = cJSON_Parse(response);
        if (root) {
            cJSON *ok = cJSON_GetObjectItemCaseSensitive(root, "success");
            const char *country = json_string(root, "country_code");
            const char *region = json_string(root, "region");
            const char *city = json_string(root, "city");
            if (cJSON_IsTrue(ok) && country[0] && (city[0] || region[0])) {
                copy_text(location->country_code, sizeof(location->country_code), country);
                copy_text(location->region, sizeof(location->region), region);
                copy_text(location->city, sizeof(location->city), city);
                success = true;
            }
            cJSON_Delete(root);
        }
    }
    std::free(response);
    return success;
}

void url_encode(const char *source, char *target, std::size_t capacity) {
    static constexpr char kHex[] = "0123456789ABCDEF";
    std::size_t out = 0;
    for (const unsigned char *p = reinterpret_cast<const unsigned char *>(source);
         p && *p && out + 1 < capacity; ++p) {
        const unsigned char value = *p;
        if (std::isalnum(value) || value == '-' || value == '_' || value == '.' ||
            value == '~') {
            target[out++] = static_cast<char>(value);
        } else if (out + 3 < capacity) {
            target[out++] = '%';
            target[out++] = kHex[value >> 4];
            target[out++] = kHex[value & 0x0F];
        } else {
            break;
        }
    }
    target[out] = '\0';
}

const char *translated_city(const RadioLocation &location) {
    if (std::strcmp(location.country_code, "CN") != 0) return location.city;
    for (const auto &entry : kChineseCities) {
        if (std::strcmp(entry.english, location.city) == 0) return entry.chinese;
    }
    return nullptr;
}

void normalize_stream_url(const char *source, char *target, std::size_t capacity) {
    if (starts_with(source, "https://") &&
        (contains(source, ".qtfm.cn/") || contains(source, ".qingting.fm/"))) {
        std::snprintf(target, capacity, "http://%s", source + 8);
    } else {
        copy_text(target, capacity, source);
    }
}

bool duplicate_url(const RadioStation *stations, std::size_t count, const char *url) {
    for (std::size_t i = 0; i < count; ++i) {
        if (std::strcmp(stations[i].url, url) == 0) return true;
    }
    return false;
}

bool ascii_only(const char *text) {
    if (!text || !text[0]) return false;
    for (const unsigned char *p = reinterpret_cast<const unsigned char *>(text); *p; ++p) {
        if (*p >= 0x80) return false;
    }
    return true;
}

// Chinese FM broadcasting occupies 87.0-108.0 MHz. Station names in the public
// directory spell the frequency in three ways, so they are tried in order of
// confidence: an explicit "FM" prefix, a bare decimal such as "94.7", and
// finally a compact form such as "971" that stations use as a brand. The last
// form is the least reliable, so it is only accepted when the digits are not
// glued to other numbers and still land inside the broadcast band.

void make_display_name(char *target, std::size_t capacity, const char *raw_name,
                       const char *city, std::size_t sequence) {
    if (ascii_only(raw_name)) {
        copy_text(target, capacity, raw_name);
        return;
    }

    const char *frequency = nullptr;
    for (const char *p = raw_name; p && p[0] && p[1]; ++p) {
        if ((p[0] == 'F' || p[0] == 'f') && (p[1] == 'M' || p[1] == 'm')) {
            frequency = p;
            break;
        }
    }
    if (frequency) {
        char suffix[20] = {};
        std::size_t length = 0;
        while (frequency[length] && length + 1 < sizeof(suffix)) {
            const unsigned char value = static_cast<unsigned char>(frequency[length]);
            if (value >= 0x80 || (!std::isalnum(value) && value != '.' && value != '-' &&
                                  value != ' ')) {
                break;
            }
            suffix[length] = frequency[length];
            ++length;
        }
        while (length > 0 && suffix[length - 1] == ' ') suffix[--length] = '\0';
        if (length >= 2) {
            if (!target || !capacity) return;
            if (capacity <= length + 1) { copy_text(target, capacity, suffix); return; }
            std::size_t kept = std::min(std::strlen(city), capacity - length - 2);
            while (kept && (static_cast<unsigned char>(city[kept]) & 0xc0) == 0x80) --kept;
            std::memcpy(target, city, kept); target[kept++] = ' ';
            std::memcpy(target + kept, suffix, length); target[kept + length] = '\0';
            return;
        }
    }
    std::snprintf(target, capacity, "%s RADIO %u", city,
                  static_cast<unsigned>(sequence + 1));
}

void append_station(const char *json, const RadioLocation &location,
                    RadioStation *stations, std::size_t capacity, std::size_t &count) {
    if(count>=capacity)return;
    std::unique_ptr<cJSON,decltype(&cJSON_Delete)> parsed(cJSON_Parse(json),cJSON_Delete);
    cJSON *entry=parsed.get();
    if(!cJSON_IsObject(entry))return;
        const char *raw_name = json_string(entry, "name");
        const char *raw_url = json_string(entry, "url_resolved");
        if (!raw_url[0]) raw_url = json_string(entry, "url");
        cJSON *last_ok = cJSON_GetObjectItemCaseSensitive(entry, "lastcheckok");
        cJSON *hls = cJSON_GetObjectItemCaseSensitive(entry, "hls");
        if (!raw_name[0] || !raw_url[0] || strlen(raw_url)>=sizeof(RadioStation::url) || !contains(raw_url, ".mp3") ||
            (cJSON_IsNumber(last_ok) && last_ok->valueint != 1) ||
            (cJSON_IsNumber(hls) && hls->valueint != 0)) {
            return;
        }

        RadioStation candidate = {};
        normalize_stream_url(raw_url, candidate.url, sizeof(candidate.url));
        if ((!starts_with(candidate.url, "http://") &&
             !starts_with(candidate.url, "https://")) ||
            duplicate_url(stations, count, candidate.url)) {
            return;
        }
        const char *city = location.city[0] ? location.city : location.region;
        candidate.frequency_decihz = radio_parse_frequency(raw_name);
        make_display_name(candidate.name, sizeof(candidate.name), raw_name, city, count);
        copy_text(candidate.description, sizeof(candidate.description),
                  "城市电台 · 网络直播");
        stations[count++] = candidate;
}

std::size_t query_directory(const RadioLocation &location, const char *field,
                            const char *value, RadioStation *stations,
                            std::size_t capacity) {
    char country[16] = {};
    char encoded[192] = {};
    url_encode(location.country_code, country, sizeof(country));
    url_encode(value, encoded, sizeof(encoded));
    char url[512] = {};
    std::snprintf(url, sizeof(url),
                  "%s?countrycode=%s&%s=%s&codec=MP3&hidebroken=true"
                  "&order=clickcount&reverse=true&limit=%u",
                  kDirectoryBase, country, field, encoded,
                  static_cast<unsigned>(std::min<std::size_t>(capacity + 3, 12)));

    // Never hold an entire directory response and its JSON tree in C3 RAM.
    auto objects=std::unique_ptr<RadioJsonObjects>(new(std::nothrow) RadioJsonObjects);
    if(!objects)return 0;
    esp_http_client_config_t config={};config.url=url;config.timeout_ms=2500;
    config.buffer_size=1024;config.buffer_size_tx=512;
    config.crt_bundle_attach=esp_crt_bundle_attach;
    config.user_agent="AI-Passport-Radio/2.0";
    esp_http_client_handle_t client=esp_http_client_init(&config);
    if(!client)return 0;
    esp_http_client_set_header(client,"Accept","application/json");
    esp_http_client_set_header(client,"Accept-Encoding","identity");
    std::size_t count=0,total=0;
    const int64_t deadline=esp_timer_get_time()+10000000;
    if(!cancelled.load()&&radio_http_open(client,3)){
        char block[256];
        while(!cancelled.load()&&!objects->done&&!objects->failed&&count<capacity&&
              total<65536&&esp_timer_get_time()<deadline){
            int received=esp_http_client_read(client,block,sizeof(block));
            if(received<=0)break;
            total+=received;
            for(int i=0;i<received&&!objects->failed;i++)
                if(objects->feed(block[i]))append_station(objects->object,location,stations,capacity,count);
        }
    }
    esp_http_client_close(client);esp_http_client_cleanup(client);
    if(objects->failed)count=0;

    return count;
}

}  // namespace

std::size_t radio_catalog_discover(RadioStation *stations, std::size_t capacity,
                                   RadioLocation *location) {
    if (!stations || capacity == 0 || !location) return 0;
    *location = {};
    if (cancelled.load() || !locate(location)) {
        ESP_LOGW(kTag, "Unable to determine network location");
        return 0;
    }
    ESP_LOGI(kTag, "Detected location: %s, %s, %s", location->city,
             location->region, location->country_code);

    std::size_t count = 0;
    const char *city_query = translated_city(*location);
    if (city_query && city_query[0]) {
        count = query_directory(*location, "name", city_query, stations, capacity);
        ESP_LOGI(kTag, "City search '%s' returned %u playable stations", city_query,
                 static_cast<unsigned>(count));
    } else if (std::strcmp(location->country_code, "CN") != 0 && location->city[0]) {
        count = query_directory(*location, "name", location->city, stations, capacity);
        ESP_LOGI(kTag, "City search '%s' returned %u playable stations", location->city,
                 static_cast<unsigned>(count));
    }

    if (count == 0 && location->region[0] && !cancelled.load()) {
        count = query_directory(*location, "state", location->region, stations, capacity);
        ESP_LOGI(kTag, "Region search '%s' returned %u playable stations",
                 location->region, static_cast<unsigned>(count));
    }
    return count;
}

std::size_t radio_catalog_discover_city(const char *city, RadioStation *stations,
                                        std::size_t capacity,
                                        RadioLocation *location) {
    if (!city || !city[0] || !stations || capacity == 0 || !location) return 0;
    *location = {};
    copy_text(location->country_code, sizeof(location->country_code), "CN");
    copy_text(location->city, sizeof(location->city), city);
    copy_text(location->region, sizeof(location->region), city);

    const std::size_t count =
        query_directory(*location, "name", city, stations, capacity);
    ESP_LOGI(kTag, "Manual city search '%s' returned %u playable stations", city,
             static_cast<unsigned>(count));
    return count;
}

void radio_catalog_cancel(){cancelled.store(true);}
void radio_catalog_reset_cancel(){cancelled.store(false);}

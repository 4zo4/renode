/**
 * @file renode_log.cpp
 * @brief Logging support
 */
#include <iostream>

#include "renode_log.h"

static uint64_t boot_ts = 0;

const char* actionToStr(Action action) {
    switch (action) {
        case invalidAction:           return "invalidAction";
        case tickClock:               return "tickClock";
        case writeRequest:            return "writeRequest";
        case readRequest:             return "readRequest";
        case resetPeripheral:         return "resetPeripheral";
        case logMessage:              return "logMessage";
        case interrupt:               return "interrupt";
        case disconnect:              return "disconnect";
        case error:                   return "error";
        case ok:                      return "ok";
        case handshake:               return "handshake";
        case pushDoubleWord:          return "pushDoubleWord";
        case getDoubleWord:           return "getDoubleWord";
        case pushWord:                return "pushWord";
        case getWord:                 return "getWord";
        case pushByte:                return "pushByte";
        case getByte:                 return "getByte";
        case isHalted:                return "isHalted";
        case registerGet:             return "registerGet";
        case registerSet:             return "registerSet";
        case singleStepMode:          return "singleStepMode";
        case readRequestByte:         return "readRequestByte";
        case readRequestWord:         return "readRequestWord";
        case readRequestDoubleWord:   return "readRequestDoubleWord";
        case readRequestQuadWord:     return "readRequestQuadWord";
        case writeRequestByte:        return "writeRequestByte";
        case writeRequestWord:        return "writeRequestWord";
        case writeRequestDoubleWord:  return "writeRequestDoubleWord";
        case writeRequestQuadWord:    return "writeRequestQuadWord";
        case pushQuadWord:            return "pushQuadWord";
        case getQuadWord:             return "getQuadWord";
        case pushConfirmation:        return "pushConfirmation";
        case step:                    return "step";
    }
    return "UNKNOWN";
}

void set_timestamp(void)
{
#ifndef WINDOWS
    if (boot_ts != 0)
        return;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t now = ((uint64_t)ts.tv_sec * 1000000ULL + (ts.tv_nsec / 1000ULL));
    setenv("BASE_TIME", std::to_string(now).c_str(), 1);
    boot_ts = now;
#endif
}

uint64_t get_timestamp(void)
{
#ifndef WINDOWS
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t now = ((uint64_t)ts.tv_sec * 1000000ULL + (ts.tv_nsec / 1000ULL));

    if (boot_ts == 0) {
        const char* base_time = getenv("BASE_TIME");
        if (base_time)
            boot_ts = std::stoull(base_time);
        else
            boot_ts = now;
    }
    return (now - boot_ts);
#else
    return boot_ts;
#endif
}

void addPrefix(const std::string& tag)
{
#ifndef WINDOWS
    uint64_t ts = get_timestamp();
    uint32_t us = (uint32_t)(ts % 1000000), tsec = (uint32_t)(ts / 1000000);
    uint32_t sec = tsec % 60, min = (tsec / 60) % 60, hour = (tsec / 3600) % 24, days = tsec / 86400;
    printf("\033[2m[%03u:%02u:%02u:%02u.%06u]\033[0m [%s] ", days, hour, min, sec, us, tag.c_str());
#else
    printf("[%s] ", tag.c_str());
#endif
}

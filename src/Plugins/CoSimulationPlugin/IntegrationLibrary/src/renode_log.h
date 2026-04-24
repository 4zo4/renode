
#ifndef RENODE_LOG_H
#define RENODE_LOG_H

#include "src/renode.h"

#if defined(ENABLE_LOGS) && (ENABLE_LOGS == 1)
#define LOG_FUNC(prefix) \
    addPrefix(prefix); \
    std::cout << __func__ << std::endl
#define LOG_ARGS(prefix, action, address, value, index) \
    addPrefix(prefix); \
    std::cout << __func__ << " Action: " << actionToStr(static_cast<Action>(action)) << " (" << action << ")" << " | Addr: 0x" \
              << std::hex << address << " | Value: 0x" << std::hex << value << " | Index: " << std::dec << index << std::endl
#else
#define LOG_FUNC(prefix) do {} while(0)
#define LOG_ARGS(prefix, action, address, value, index) do {} while(0)
#endif

void set_timestamp(void);
void addPrefix(const std::string& tag);
const char* actionToStr(Action action);

#endif
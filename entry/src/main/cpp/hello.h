//
// Created on 2024/4/29.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef NAPIDEMO_HELLO_H
#define NAPIDEMO_HELLO_H

#include "napi/native_api.h"
#include <assert.h>
#include <cstring>

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0x0001
#define LOG_TAG "NapiDemo"
#include <hilog/log.h>

#define ARLEN(a) sizeof(a) / sizeof(a[0])

#endif //NAPIDEMO_HELLO_H

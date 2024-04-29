//
// Created on 2024/4/29.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef NAPIDEMO_ASYNC_H
#define NAPIDEMO_ASYNC_H

#include "napi/native_api.h"

napi_value producePromise(napi_env env, napi_callback_info info);

#endif //NAPIDEMO_ASYNC_H
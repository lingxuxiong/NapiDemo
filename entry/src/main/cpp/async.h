//
// Created on 2024/4/29.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef NAPIDEMO_ASYNC_H
#define NAPIDEMO_ASYNC_H

#include "napi/native_api.h"

struct CallbackData {
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;
    int args = 0;
    int result = 0;
};

void ExecuteCB(napi_env env, void *data);
void CompleteCB(napi_env env, napi_status status, void *data);

#endif //NAPIDEMO_ASYNC_H
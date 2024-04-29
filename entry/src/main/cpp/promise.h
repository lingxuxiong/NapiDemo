//
// Created on 2024/4/29.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef NAPIDEMO_PROMISE_H
#define NAPIDEMO_PROMISE_H

#include "napi/native_api.h"

typedef napi_value (*PromiseHandler)(napi_env env, napi_callback_info info);

napi_value handlePromise(napi_env env, napi_value promise, PromiseHandler resolve, PromiseHandler reject);

napi_value producePromise(napi_env env, napi_callback_info info);
napi_value consumePromise(napi_env env, napi_callback_info info);

#endif //NAPIDEMO_PROMISE_H
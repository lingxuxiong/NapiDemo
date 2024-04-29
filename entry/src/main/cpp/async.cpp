//
// Created on 2024/4/29.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "async.h"
#include <hilog/log.h>

void ExecuteCB(napi_env env, void *data) 
{
    CallbackData *callbackData = reinterpret_cast<CallbackData *>(data);
    callbackData->result = callbackData->args;
    OH_LOG_DEBUG(LOG_APP, "async task ended up with result: %{public}d", callbackData->result);
}

void CompleteCB(napi_env env, napi_status status, void *data) 
{
    CallbackData *callbackData = reinterpret_cast<CallbackData *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, callbackData->result, &result);
    callbackData->result > 0 ? napi_resolve_deferred(env, callbackData->deferred, result)
                             : napi_reject_deferred(env, callbackData->deferred, result);

    napi_delete_async_work(env, callbackData->asyncWork);

    OH_LOG_DEBUG(LOG_APP, "complete async task");

    delete callbackData;
}

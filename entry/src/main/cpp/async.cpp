//
// Created on 2024/4/29.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "hello.h"

struct CallbackData 
{
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;
    int args = 0;
    int result = 0;
};

static void ExecuteCB(napi_env env, void *data) 
{
    CallbackData *callbackData = reinterpret_cast<CallbackData *>(data);
    callbackData->result = callbackData->args;
    OH_LOG_DEBUG(LOG_APP, "async task ended up with result: %{public}d", callbackData->result);
}

static void CompleteCB(napi_env env, napi_status status, void *data) 
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

napi_value producePromise(napi_env env, napi_callback_info info) 
{
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    napi_create_promise(env, &deferred, &promise);

    auto callbackData = new CallbackData();
    callbackData->deferred = deferred;
    napi_get_value_int32(env, args[0], &callbackData->args);

    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "AsyncCallback", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env, nullptr, resourceName, ExecuteCB, CompleteCB, callbackData, &callbackData->asyncWork);
    napi_queue_async_work(env, callbackData->asyncWork);

    OH_LOG_DEBUG(LOG_APP, "created async task");

    return promise;
}

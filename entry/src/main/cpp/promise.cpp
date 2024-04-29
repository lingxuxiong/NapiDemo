//
// Created on 2024/4/29.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "hello.h"
#include "promise.h"
#include "async.h"
#include <cassert>
#include <cstdio>
#include <hilog/log.h>

napi_value producePromise(napi_env env, napi_callback_info info) {    
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

napi_value consumePromise(napi_env env, napi_callback_info info) {
    
    napi_status status;

    size_t argc = 2;
    napi_value args[2] = {nullptr};
    status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    assert(status == napi_ok);

    napi_value arg = args[0];
    int num = 0;
    status = napi_get_value_int32(env, arg, &num);
    assert(status == napi_ok);
    OH_LOG_DEBUG(LOG_APP, "got num arg with value: %{public}d", num);

    napi_value callback = args[1];
    napi_value doubledNum = nullptr;
    status = napi_create_int32(env, 2 * num, &doubledNum);
    assert(status == napi_ok);

    // Get the promise returned from the JS side
    napi_value cbArgs[] = { doubledNum };
    napi_value promise = nullptr;
    status = napi_call_function(env, nullptr, callback, ARLEN(cbArgs), cbArgs, &promise);
    assert(status == napi_ok);

    bool isPromise = false;
    napi_is_promise(env, promise, &isPromise);
    OH_LOG_DEBUG(LOG_APP, "isPromise: %{public}d", isPromise);

    return handlePromise(env, promise, resolvePromise, rejectPromise);
}

napi_value resolvePromise(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    size_t len = 0;
    char str[256] = {'\0'};
    napi_get_value_string_utf8(env, args[0], str, ARLEN(str), &len);
    OH_LOG_DEBUG(LOG_APP, "got formatted string %{public}s", str);
    return nullptr;
}

napi_value rejectPromise(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    size_t len = 0;
    char errorMessage[256] = {'\0'};
    napi_get_value_string_utf8(env, args[0], errorMessage, ARLEN(errorMessage), &len);
    OH_LOG_DEBUG(LOG_APP, "got error message: %{public}s", errorMessage);

    return nullptr;
}

napi_value handlePromise(napi_env env, napi_value promise, PromiseHandler resolve, PromiseHandler reject) {
    bool isPromise = false;
    napi_is_promise(env, promise, &isPromise);
    if (!isPromise) {
        OH_LOG_DEBUG(LOG_APP, "not a promise");
        return nullptr;
    }

    ///////////// Prepare to consume the promise /////////////

    napi_status status;

    // 1. Get the then property of the target promise
    napi_value thenProp;
    status = napi_get_named_property(env, promise, "then", &thenProp);
    assert(status == napi_ok);

    // 2. Create the then function and associate it with the function to be called,
    // so as to handle the data returned from the JS side.
    napi_value thenFunc;
    status = napi_create_function(env, "thenFunc", NAPI_AUTO_LENGTH, resolve, nullptr, &thenFunc);
    assert(status == napi_ok);

    // 3. Call the then function and return the call result
    napi_value result;
    status = napi_call_function(env, promise, thenProp, 1, &thenFunc, &result);
    assert(status == napi_ok);
    OH_LOG_DEBUG(LOG_APP, "then function was called from native");

    // 4. Get the catch property for potential promise errors
    napi_value catchProp;
    napi_get_named_property(env, promise, "catch", &catchProp);

    // 5. Create the catch function and associate it with the function to be called,
    // so as to handle the error generated from the JS side.
    napi_value catchFunc;
    napi_create_function(env, "catchFunc", NAPI_AUTO_LENGTH, reject, nullptr, &catchFunc);

    // 6. Call the catch function for potential errors.
    napi_call_function(env, promise, catchProp, 1, &catchFunc, &result);

    return nullptr;
}
#include "napi/native_api.h"
#include <assert.h>

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0x0001
#define LOG_TAG "NapiDemo"
#include <hilog/log.h>

#define ARLEN(a) sizeof(a)/sizeof(a[0])

typedef napi_value (*CALLBACK)(napi_env env, napi_callback_info info);

struct CallbackData {
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;
    int args = 0;
    int result = 0;
};

static void ExecuteCB(napi_env env, void *data) {
    CallbackData *callbackData = reinterpret_cast<CallbackData *>(data);
    callbackData->result = callbackData->args;
    OH_LOG_DEBUG(LOG_APP, "async task ended up with result: %{public}d", callbackData->result);
}

static void CompleteCB(napi_env env, napi_status status, void *data) {
    CallbackData *callbackData = reinterpret_cast<CallbackData *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, callbackData->result, &result);
    callbackData->result > 0 ? napi_resolve_deferred(env, callbackData->deferred, result)
                             : napi_reject_deferred(env, callbackData->deferred, result);

    napi_delete_async_work(env, callbackData->asyncWork);

    OH_LOG_DEBUG(LOG_APP, "complete async task");

    delete callbackData;
}

static napi_value AsyncWork(napi_env env, napi_callback_info info) {
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

static void resolve_callback(napi_env env, napi_value result, void *data) {
    napi_typedarray_type type;
    napi_value buf;
    size_t offset;
    size_t len;
    napi_status status = napi_get_typedarray_info(env, result, &type, &len, &data, &buf, &offset);
    // assert(status == napi_ok);
    OH_LOG_DEBUG(LOG_APP, "file data len: %{public}zu, type: %{public}d, offset: %{public}zu", len, type, offset);

    if (type == napi_uint8_array) {
        uint8_t *dataArr = static_cast<uint8_t *>(data);
        for (size_t i = 0; i < len; ++i) {
            OH_LOG_DEBUG(LOG_APP, "data[%{public}zu]: %{public}d", i, dataArr[i]);
        }
    }
}

static void reject_callback(napi_env env, napi_value error, void *data) {
    char buffer[128];
    size_t buffer_size = 128;
    size_t copied;

    napi_status status = napi_get_value_string_utf8(env, error, buffer, buffer_size, &copied);
    assert(status == napi_ok);

    OH_LOG_DEBUG(LOG_APP, "Promise rejected with error: %s\n", buffer);
}

// Callback function for Promise resolution
static napi_value ResolveCallback(napi_env env, napi_callback_info info) {
    double result = 1.0;
    OH_LOG_DEBUG(LOG_APP, "promise resolved with: %{public}f", result);
}

// Callback function for Promise rejection
static napi_value RejectCallback(napi_env env, napi_callback_info info) {
    OH_LOG_DEBUG(LOG_APP, "promise gets rejected");
}

static napi_value Add(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_get_cb_info(env, info, &argc, args , nullptr, nullptr);

    napi_valuetype valuetype0;
    napi_typeof(env, args[0], &valuetype0);

    napi_valuetype valuetype1;
    napi_typeof(env, args[1], &valuetype1);

    double value0;
    napi_get_value_double(env, args[0], &value0);

    double value1;
    napi_get_value_double(env, args[1], &value1);

    napi_value sum;
    napi_create_double(env, value0 + value1, &sum);

    return sum;

}

static napi_value resolveCallback(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    size_t len = 0;
    char str[256] = {'\0'};
    napi_get_value_string_utf8(env, args[0], str, ARLEN(str), &len);
    OH_LOG_DEBUG(LOG_APP, "got formatted string %{public}s", str);
    return nullptr;
}

static napi_value rejectCallback(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    size_t len = 0;
    char errorMessage[256] = {'\0'};
    napi_get_value_string_utf8(env, args[0], errorMessage, ARLEN(errorMessage), &len);
    OH_LOG_DEBUG(LOG_APP, "got error message: %{public}s", errorMessage);

    return nullptr;
}

static napi_value handlePromise(napi_env env, napi_value promise, CALLBACK success, CALLBACK error) 
{
    bool isPromise = false;
    napi_is_promise(env, promise, &isPromise);
    OH_LOG_DEBUG(LOG_APP, "isPromise: %{public}d", isPromise);
    if (!isPromise) {
        return nullptr;
    }

    ///////////// Prepare to consume the promise /////////////

    // 1. Get the then property of the target promise
    napi_value promiseThen;
    napi_get_named_property(env, promise, "then", &promiseThen);

    // 2. Create the then function of the promise
    napi_value promiseThenFunc;
    napi_create_function(env, "thenFunc", NAPI_AUTO_LENGTH, success, nullptr, &promiseThenFunc);

    // 3. Call the then function and return the call result
    napi_value promiseResult;
    napi_call_function(env, promise, promiseThen, 1, &promiseThenFunc, &promiseResult);


    // 3. Catch promise error
    napi_value promiseCatch;
    napi_get_named_property(env, promise, "catch", &promiseCatch);

    // 4. Create the catch function of the promise
    napi_value promiseCatchFunc;
    napi_create_function(env, "catchFunc", NAPI_AUTO_LENGTH, error, nullptr, &promiseCatchFunc);

    // 5. Call the catch function and return the some error was caught
    napi_call_function(env, promise, promiseCatch, 1, &promiseCatchFunc, &promiseResult);

    return 0;
}

static napi_value Promise(napi_env env, napi_callback_info info) { 
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
    
    return handlePromise(env, promise, resolveCallback, rejectCallback);
}


static napi_value Print(napi_env env, napi_callback_info info) {

        napi_status status;

        size_t argc = 3;
        napi_value args[3] = {nullptr};
        status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        assert(status == napi_ok);

        napi_value val = args[0];
        int id = 0;
        status = napi_get_value_int32(env, val, &id);
        assert(status == napi_ok);
        OH_LOG_DEBUG(LOG_APP, "id: %{public}d", id);

        napi_value filePathCallback = args[1];
        int32_t num = 1;
        napi_value pageNumArg;
        status = napi_create_int32(env, num, &pageNumArg);
        assert(status == napi_ok);

        napi_value filePathCallbackArgs[] = {pageNumArg};
        napi_value resultPromise = nullptr;
        status = napi_call_function(env, nullptr, filePathCallback, ARLEN(filePathCallbackArgs), filePathCallbackArgs,
                                    &resultPromise);
        assert(status == napi_ok);


        auto successCbA = [](napi_env env, napi_callback_info info) -> napi_value {
            size_t argc = 1;
            napi_value args[1] = {nullptr};
            napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
            char filePath[256] = {'\0'};
            size_t len = 0;
            napi_get_value_string_utf8(env, args[0], filePath, 256, &len);
            OH_LOG_DEBUG(LOG_APP, "got file path for page %{public}s", filePath);

            napi_value undefined;
            napi_get_undefined(env, &undefined);
            return undefined;
        };

        napi_value promiseThen;
        napi_get_named_property(env, resultPromise, "then", &promiseThen);
        napi_value successFunA;
        napi_create_function(env, "successFunc", NAPI_AUTO_LENGTH, successCbA, nullptr, &successFunA);
        napi_value ret;
        napi_call_function(env, resultPromise, promiseThen, 1, &successFunA, &ret);


        //     char filePath[256] = {'\0'};
        size_t len = 0;
        //     status = napi_get_value_string_utf8(env, result, filePath, 256, &len);
        //     assert(status == napi_ok);
        //     OH_LOG_DEBUG(LOG_APP, "got file path for page %{public}d: %{public}s", pageNum, filePath);

        //     napi_value fileDataCallback = args[2];
        //     napi_value fileDataCallbackArgs[] = { result };
        //     cbArgc = sizeof(fileDataCallbackArgs) / sizeof(fileDataCallbackArgs[0]);
        //     status = napi_call_function(env, nullptr, fileDataCallback, cbArgc, fileDataCallbackArgs, &result);
        //     assert(status == napi_ok);
        //
        //     napi_typedarray_type type;
        //     napi_value buf;
        //     size_t offset;
        //     void *data;
        //     status = napi_get_typedarray_info(env, result, &type, &len, &data, &buf, &offset);
        //     //assert(status == napi_ok);
        //     OH_LOG_DEBUG(LOG_APP, "file data len: %{public}zu, type: %{public}d, offset: %{public}zu", len, type,
        //     offset);
        //
        //     if (type == napi_uint8_array) {
        //         uint8_t *dataArr = static_cast<uint8_t *>(data);
        //         for (size_t i = 0; i < len; ++i) {
        //             OH_LOG_DEBUG(LOG_APP, "data[%{public}zu]: %{public}d", i, dataArr[i]);
        //         }
        //     }

        return 0;
    }

    EXTERN_C_START
    static napi_value Init(napi_env env, napi_value exports) {
        napi_property_descriptor desc[] = {
            {"add", nullptr, Add, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"print", nullptr, Print, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"asyncWork", nullptr, AsyncWork, nullptr, nullptr, nullptr, napi_default, nullptr},
            {"callWithPromise", nullptr, Promise, nullptr, nullptr, nullptr, napi_default, nullptr}};
        napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
        return exports;
    }
    EXTERN_C_END

    static napi_module demoModule = {
        .nm_version = 1,
        .nm_flags = 0,
        .nm_filename = nullptr,
        .nm_register_func = Init,
        .nm_modname = "entry",
        .nm_priv = ((void *)0),
        .reserved = {0},
    };

    extern "C" __attribute__((constructor)) void RegisterEntryModule(void) { napi_module_register(&demoModule); }

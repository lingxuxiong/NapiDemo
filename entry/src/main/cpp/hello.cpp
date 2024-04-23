#include "napi/native_api.h"
#include <assert.h>

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0x0001
#define LOG_TAG "NapiDemo"
#include <hilog/log.h>

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
    int32_t pageNum = 1;
    napi_value pageArg;
    status = napi_create_int32(env, pageNum, &pageArg);
    assert(status == napi_ok);
    
    napi_value filePathCallbackArgs[] = { pageArg };
    size_t cbArgc = sizeof(filePathCallbackArgs) / sizeof(filePathCallbackArgs[0]);
    napi_value result = nullptr;
    status = napi_call_function(env, nullptr, filePathCallback, cbArgc, filePathCallbackArgs, &result);
    assert(status == napi_ok);
    
    char filePath[256] = {'\0'};
    size_t len = 0;
    status = napi_get_value_string_utf8(env, result, filePath, 256, &len);
    assert(status == napi_ok);
    OH_LOG_DEBUG(LOG_APP, "got file path for page %{public}d: %{public}s", pageNum, filePath);
    
    napi_value fileDataCallback = args[2];
    napi_value fileDataCallbackArgs[] = { result };
    cbArgc = sizeof(fileDataCallbackArgs) / sizeof(fileDataCallbackArgs[0]);
    status = napi_call_function(env, nullptr, fileDataCallback, cbArgc, fileDataCallbackArgs, &result);
    assert(status == napi_ok);

    napi_typedarray_type type;
    napi_value buf;
    size_t offset;
    void *data;
    status = napi_get_typedarray_info(env, result, &type, &len, &data, &buf, &offset);
    //assert(status == napi_ok);
    OH_LOG_DEBUG(LOG_APP, "file data len: %{public}zu, type: %{public}d, offset: %{public}zu", len, type, offset);

    if (type == napi_uint8_array) {
        uint8_t *dataArr = static_cast<uint8_t *>(data);
        for (size_t i = 0; i < len; ++i) {
            OH_LOG_DEBUG(LOG_APP, "data[%{public}zu]: %{public}d", i, dataArr[i]);
        }
    }
    
    return 0;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        { "add", nullptr, Add, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "print", nullptr, Print, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "asyncWork", nullptr, AsyncWork, nullptr, nullptr, nullptr, napi_default, nullptr }
    };
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
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void)
{
    napi_module_register(&demoModule);
}

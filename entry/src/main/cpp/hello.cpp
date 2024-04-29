#include "hello.h"
#include "async.h"
#include <cstddef>
#include <cstdlib>

typedef napi_value (*PromiseHandler)(napi_env env, napi_callback_info info);

static napi_value handleFileDataPromise(napi_env env, napi_callback_info info) 
{
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    size_t len;
    void *data;    
    napi_status status = napi_get_arraybuffer_info(env, args[0], &data, &len);
    assert(status == napi_ok);

    if (data != nullptr) {
        uint8_t *dataArr = static_cast<uint8_t *>(data);
        for (size_t i = 0; i < 100; ++i) {
            OH_LOG_DEBUG(LOG_APP, "data[%{public}zu]: %{public}d", i, dataArr[i]);
        }
    }
    
    return 0;
}

static napi_value resolvePromise(napi_env env, napi_callback_info info) 
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    size_t len = 0;
    char str[256] = {'\0'};
    napi_get_value_string_utf8(env, args[0], str, ARLEN(str), &len);
    OH_LOG_DEBUG(LOG_APP, "got formatted string %{public}s", str);
    return nullptr;
}

static char sFilePath[256] = "/data/storage/el2/base/haps/entry/files/sample.jpg";
static napi_value resolveFilePathPromise(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    size_t len = 0;
    napi_get_value_string_utf8(env, args[0], sFilePath, ARLEN(sFilePath), &len);
    OH_LOG_DEBUG(LOG_APP, "got file path: %{public}s", sFilePath);
    return nullptr;
}

static napi_value rejectPromise(napi_env env, napi_callback_info info) 
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    size_t len = 0;
    char errorMessage[256] = {'\0'};
    napi_get_value_string_utf8(env, args[0], errorMessage, ARLEN(errorMessage), &len);
    OH_LOG_DEBUG(LOG_APP, "got error message: %{public}s", errorMessage);

    return nullptr;
}

static napi_value handlePromise(napi_env env, napi_value promise, PromiseHandler resolve, PromiseHandler reject) 
{
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

static napi_value consumePromise(napi_env env, napi_callback_info info) 
{ 
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


static napi_value Print(napi_env env, napi_callback_info info) 
{
    napi_status status;

    size_t argc = 3;
    napi_value args[3] = {nullptr};
    status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    assert(status == napi_ok);

    napi_value id = args[0];
    napi_value filePathCallback = args[1];
    napi_value fileDataCallback = args[2];
    
    int identifier = 0;
    status = napi_get_value_int32(env, id, &identifier);
    assert(status == napi_ok);
    OH_LOG_DEBUG(LOG_APP, "id: %{public}d", identifier);
    
    int32_t pageNum = rand() % 100;
    napi_value pageNumArg;
    status = napi_create_int32(env, pageNum, &pageNumArg);
    assert(status == napi_ok);
    
    napi_value filePathCallbackArgs[] = { pageNumArg };
    napi_value filePathPromise = nullptr;    
    status = napi_call_function(env, nullptr, filePathCallback, 
    ARLEN(filePathCallbackArgs), filePathCallbackArgs,&filePathPromise);
    assert(status == napi_ok);

    napi_value result = handlePromise(env, filePathPromise, 
    resolveFilePathPromise,rejectPromise);
    
    OH_LOG_DEBUG(LOG_APP, "current file path: %{public}s", sFilePath);
    size_t len = strlen(sFilePath);
    assert(len != 0);
    napi_value filePathArg;
    status = napi_create_string_utf8(env, sFilePath, len, &filePathArg);
    assert(status == napi_ok);
    
    napi_value fileDataCallbackArgs[] = { filePathArg };
    napi_value fileDataPromise = nullptr;
    status = napi_call_function(env, nullptr, fileDataCallback, 
    ARLEN(fileDataCallbackArgs), fileDataCallbackArgs, &fileDataPromise);
    assert(status == napi_ok);

    result = handlePromise(env, fileDataPromise, 
    handleFileDataPromise,rejectPromise);

    return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) 
{
    napi_property_descriptor desc[] = {
        {"print", nullptr, Print, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"producePromise", nullptr, producePromise, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"consumePromise", nullptr, consumePromise, nullptr, nullptr, nullptr, napi_default, nullptr}};
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = 
{
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "entry",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void) { napi_module_register(&demoModule); }

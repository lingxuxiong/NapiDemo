#include "hello.h"
#include "promise.h"
#include <cstddef>
#include <cstdlib>
#include <hilog/log.h>
#include <cstring>

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

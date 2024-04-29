#include "hello.h"
#include "async.h"

typedef napi_value (*CALLBACK)(napi_env env, napi_callback_info info);

static napi_value printImageData(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    size_t len;
    void *data;    
    napi_status status = napi_get_arraybuffer_info(env, args[0], &data, &len);
    assert(status == napi_ok);
    OH_LOG_DEBUG(LOG_APP, "file data len: %{public}zu", len);

    if (data != nullptr) {
        uint8_t *dataArr = static_cast<uint8_t *>(data);
        for (size_t i = 0; i < 100; ++i) {
            OH_LOG_DEBUG(LOG_APP, "data[%{public}zu]: %{public}d", i, dataArr[i]);
        }
    }
    
    return 0;
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
    return 0;
}

// Callback function for Promise rejection
static napi_value RejectCallback(napi_env env, napi_callback_info info) {
    OH_LOG_DEBUG(LOG_APP, "promise gets rejected");
    return 0;
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

        napi_value ret;

        napi_value filePathCallbackArgs[] = {pageNumArg};
        napi_value filePathPromise = nullptr;
        status = napi_call_function(env, nullptr, filePathCallback, ARLEN(filePathCallbackArgs),
                                    filePathCallbackArgs,&filePathPromise);
        assert(status == napi_ok);

        ret = handlePromise(
            env, filePathPromise,
            [](napi_env env, napi_callback_info info) -> napi_value {
                size_t argc = 1;
                napi_value args[1] = {nullptr};
                napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
                char filePath[256] = {'\0'};
                size_t len = 0;
                napi_get_value_string_utf8(env, args[0], filePath, 256, &len);
                OH_LOG_DEBUG(LOG_APP, "got file path for page %{public}s", filePath);
                return nullptr;
            }, rejectCallback);

        napi_value filePathArg;
        const char *filePath = "/data/storage/el2/base/haps/entry/files/sample.jpg";
        status = napi_create_string_utf8(env, filePath, strlen(filePath), &filePathArg);
        assert(status == napi_ok);

        napi_value fileDataCallback = args[2];
        napi_value fileDataCallbackArgs[] = { filePathArg };
        napi_value fileDataPromise = nullptr;
        status = napi_call_function(env, nullptr, fileDataCallback, ARLEN(fileDataCallbackArgs), 
                                    fileDataCallbackArgs, &fileDataPromise);
        assert(status == napi_ok);

        ret = handlePromise(env, fileDataPromise, printImageData,rejectCallback);

        return ret;
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

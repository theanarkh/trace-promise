#include <node.h>
#include <v8.h>

using v8::StackFrame;
using v8::Context;
using v8::Number;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Function;
using v8::Object;
using v8::Private;
using v8::Promise;
using v8::PromiseHookType;
using v8::StackTrace;
using v8::String;
using v8::Uint32;
using v8::Value;

v8::Persistent<v8::Function> initCallback;
v8::Persistent<v8::Function> resolveCallback;
static int trace_promise_id_counter = 1;

static inline v8::Local<v8::String> v8_str(const char* str) {
  return v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), str).ToLocalChecked();
}

static uint32_t GetPromiseId(Local<Context> context, Local<Promise> promise) {
  Local<Value> id;
  if (!promise->GetPrivate(context, v8::Private::ForApi(Isolate::GetCurrent(), v8_str("trace_id")))
           .ToLocal(&id) ||
      !id->IsUint32()) {
    return 0;
  }
  return id.As<Uint32>()->Value();
}

void TracePromises(PromiseHookType type,
                   Local<Promise> promise,
                   Local<Value> parent) {
  if (type == PromiseHookType::kBefore || type == PromiseHookType::kAfter) {
    return;
  }
  Isolate* isolate = Isolate::GetCurrent();
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> obj = Object::New(isolate);
  uint32_t parent_id = 0;
  if (!parent.IsEmpty() && parent->IsPromise()) {
    parent_id = GetPromiseId(context, parent.As<Promise>());
  }

  uint32_t id = 0;
  if (type == PromiseHookType::kInit) {
    id = trace_promise_id_counter++;
    promise->SetPrivate(context,
                        v8::Private::ForApi(Isolate::GetCurrent(), v8_str("trace_id")),
                        Uint32::New(isolate, id));
  } else if (type == PromiseHookType::kResolve) {
    id = GetPromiseId(context, promise);
    if (!id) {
      return;
    }
  } else {
    return;
  }

  (void)obj->Set(context, v8_str("id"), Number::New(isolate, id));
  if (parent_id) {
    (void)obj->Set(context, v8_str("pid"), Number::New(isolate, parent_id));
  }

  Local<StackTrace> stack = StackTrace::CurrentStackTrace(isolate, 1, {});
  if (stack->GetFrameCount() != 0) {
    Local<StackFrame> frame = stack->GetFrame(isolate, 0);
    Local<String> script = frame->GetScriptName();
    int line = frame->GetLineNumber();
    (void)obj->Set(context, v8_str("file"), script);
    (void)obj->Set(context, v8_str("line"), Number::New(isolate, line));
  }
  
  Local<Value> params[] = { obj };
  Local<Function> fn = type == PromiseHookType::kInit ? initCallback.Get(isolate) : resolveCallback.Get(isolate);
  (void)fn->Call(context, context->Global(), 1, params);
}


void SetCallback(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() > 0) {
    if (args[0]->IsFunction()) {
      initCallback.Reset(isolate, args[0].As<Function>());
    }
    if (args[1]->IsFunction()) {
      resolveCallback.Reset(isolate, args[1].As<Function>());
    }
    isolate->SetPromiseHook(TracePromises);
  } else {
    isolate->SetPromiseHook(nullptr);
    initCallback.Reset();
    resolveCallback.Reset();
  }
}

void init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
  NODE_SET_METHOD(exports, "setCallback", SetCallback);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, init)

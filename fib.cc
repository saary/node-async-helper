#include <node.h>
#include <v8.h>
#include "node-async.h"

using namespace v8;
using namespace NodeUtils;

Handle<Value> DoFib(const Arguments& args) 
{
    HandleScope scope;

    if (args.Length() > 0) 
    {
        std::shared_ptr<int> num(new int(args[0]->Int32Value()));
        Async::Run<int, int>(
            num,
            [] (Async::Baton<int, int>* baton) 
            {
                auto target = *baton->data;
                auto prev = 0;
                auto curr = 1;
                for(int i=0; i<target; i++)
                {
                    int tmp = curr;
                    curr = curr + prev;
                    prev = tmp;
                }

                baton->result = std::shared_ptr<int>(new int(curr));
            },
            [] (Handle<Function> callback, Async::Baton<int, int>* baton) 
            {
                HandleScope scope;


                if (baton->error_code)
                {
                    Local<Value> err = Exception::Error(String::New(baton->error_message.c_str()));
                    Local<Value> argv[] = { err };
                    callback->Call(Context::GetCurrent()->Global(), _countof(argv), argv);
                }
                else
                {
                    Local<Value> argv[] = { Local<Value>::New(Undefined()), Integer::New(*baton->result) };
                    callback->Call(Context::GetCurrent()->Global(), _countof(argv), argv);
                }
            },
            args[1].As<Function>());
    }

    Async::RunOnMain(new std::function<void ()>([] {
      HandleScope scope;

      auto console = Context::GetCurrent()->Global()->Get(String::New(L"console")).As<Object>();
      auto log = console->Get(String::NewSymbol("log")).As<Function>();
      Local<Value> argv[] = { String::New(L"Bazinq") };

      log->Call(console, _countof(argv), argv);
    }));

    return scope.Close(Undefined());
}

void init(Handle<Object> exports) {
    exports->Set(String::NewSymbol("run"),
        FunctionTemplate::New(DoFib)->GetFunction());
}

NODE_MODULE(fib, init)
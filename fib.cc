// Copyright (c) Microsoft Corporation
// All rights reserved. 
//
// Licensed under the Apache License, Version 2.0 (the ""License""); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 
//
// THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT. 
//
// See the Apache Version 2.0 License for specific language governing permissions and limitations under the License.

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

                Async::RunOnMain([curr] {
                  HandleScope scope;  
                  auto console = Context::GetCurrent()->Global()->Get(String::New(L"console")).As<Object>();
                  auto log = console->Get(String::NewSymbol("log")).As<Function>();
                  Local<Value> argv[] = { String::New(L"Result is:"), Integer::New(curr)};

                  log->Call(console, _countof(argv), argv);
                });
            },
            [] (Async::Baton<int, int>* baton) 
            {
                HandleScope scope;

                if (baton->error_code)
                {
                    Local<Value> err = Exception::Error(String::New(baton->error_message.c_str()));
                    Handle<Value> argv[] = { err };
                    baton->setCallbackArgs(argv, _countof(argv));
                }
                else
                {
                    Handle<Value> argv[] = { Undefined(), Integer::New(*baton->result) };
                    baton->setCallbackArgs(argv, _countof(argv));
                }
            },
            args[1].As<Function>());
    }

    return scope.Close(Undefined());
}

void init(Handle<Object> exports) {
    exports->Set(String::NewSymbol("run"),
        FunctionTemplate::New(DoFib)->GetFunction());
}

NODE_MODULE(fib, init)
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
#include "nan.h"
#include "node-async.h"

using Nan::New;
using Nan::HandleScope;
using Nan::Undefined;
using v8::Integer;
using v8::Function;
using v8::FunctionTemplate;
using v8::String;
using v8::Exception;
using v8::Object;

using namespace NodeUtils;

NAN_METHOD(DoFib)
{
	HandleScope scope;

    if (info.Length() > 0) 
    {
        std::shared_ptr<int> num(new int(info[0]->Int32Value()));
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

                  auto console = Nan::GetCurrentContext()->Global()->Get(New<String>("console").ToLocalChecked()).As<Object>();
                  auto log = console->Get(New<String>("log").ToLocalChecked()).As<Function>();
                  Local<Value> argv[] = { New<String>(L"Result is:").ToLocalChecked(), New<v8::Integer>(curr)};

                  log->Call(console, _countof(argv), argv);
                });
            },
            [] (Async::Baton<int, int>* baton) 
            {
                HandleScope scope;

                if (baton->error_code)
                {
                    Local<Value> err = Exception::Error(New<String>(baton->error_message.c_str()).ToLocalChecked());
                    Handle<Value> argv[] = { err };
                    baton->setCallbackArgs(argv, _countof(argv));
                }
                else
                {
                    Handle<Value> argv[] = { Undefined(), New<Integer>(*baton->result) };
                    baton->setCallbackArgs(argv, _countof(argv));
                }
            },
            info[1].As<Function>());
    }
}

void init(Handle<Object> exports) {
    exports->Set(New<String>("run").ToLocalChecked(),
        New<FunctionTemplate>(DoFib)->GetFunction());
}

NODE_MODULE(fib, init)
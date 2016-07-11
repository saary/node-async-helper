node-async-helper
=================

Node addon async helper. Use C++ lambdas to run async code in node.

#Dependencies:

 nan - https://github.com/nodejs/nan
 
 If not already installed, make sure to install by running:
 
```
npm install nan
```

#Example usage
An async fibunacci implementation.

```C++
#include "node-async.h"

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

  return scope.Close(Undefined());
}
```

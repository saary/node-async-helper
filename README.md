node-async-helper
=================

Node addon async helper. Use C++ lambdas to run async code in node.

#Example usage
An async fibunacci implementation.

```C++
Handle<Value> DoFib(const Arguments& args) 
{
  HandleScope scope;

  if (args.Length() > 0) 
  {
    std::shared_ptr<int> num(new int(args[0]->Int32Value()));
    Async<int, int>::Run(
      num,
      [] (Async<int, int>::Baton* baton) 
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
      [] (Handle<Function> callback, Async<int, int>::Baton* baton) 
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

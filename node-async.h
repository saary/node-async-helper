#pragma once
#include <v8.h>
#include <functional>
#include <memory>

namespace NodeUtils
{
    using namespace v8;

    template<typename TInput, typename TResult> 
    class Async 
    {
    public:
        struct Baton {
            int error_code;
            std::string error_message;

            // Custom data
            std::shared_ptr<TInput> data;
            std::shared_ptr<TResult> result;

        private:
            uv_work_t request;
            std::function<void (Baton*)> doWork;
            std::function<void (Handle<Function> callback, Baton*)> afterWork;
            Persistent<Function> callback;

            friend Async<TInput, TResult>;
        };

        static void Run(
            std::shared_ptr<TInput> input, 
            std::function<void (Baton*)> doWork, 
            std::function<void (Handle<Function> callback, Baton*)> 
            afterWork, Handle<Function> callback)
        {
            HandleScope scope;

            Baton* baton = new Baton();
            baton->request.data = baton;
            baton->callback = Persistent<Function>::New(callback);
            baton->error_code = 0;
            baton->data = input;
            baton->doWork = doWork;
            baton->afterWork = afterWork;

            uv_queue_work(uv_default_loop(), &baton->request, AsyncWork, AsyncAfter);      
        }

    private:
        static void AsyncWork(uv_work_t* req) 
        {
            // No HandleScope!

            Baton* baton = static_cast<Baton*>(req->data);

            // Do work in threadpool here.
            // Set baton->error_code/message on failures.
            // Set baton->result with a final result object
            baton->doWork(baton);
        }

        static void AsyncAfter(uv_work_t* req, int status) 
        {
            HandleScope scope;
            Baton* baton = static_cast<Baton*>(req->data);

            // typical AfterWorkFunc implementation
            //if (baton->error) 
            //{
            //    // Call callback with error object.
            //} 
            //else 
            //{
            //    // Call callback with results.
            //}

            baton->afterWork(baton->callback, baton);

            baton->callback.Dispose();
            delete baton;
        }
    };
}
//////////////////////////////////////////////////////////////////////////////
//
// License Agreement:
//
// The following are Copyright © 2008, Daniel Önnerby
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the author nor the names of other contributors may
//      be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////////
// adapted from musik::core.

#ifndef __C_AUTOM8_PLUGIN_SYSTEM_PLUGIN_FACTORY_HPP__
#define __C_AUTOM8_PLUGIN_SYSTEM_PLUGIN_FACTORY_HPP__

#include <autom8/plugin/plugin.hpp>

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

namespace autom8 {

    class plugin_factory{
    public:

        static plugin_factory& instance() {
            return instance_;
        }

    private:

#if defined(WIN32)
        typedef plugin* (__stdcall* call_get_plugin)();
#elif defined (UNIX)
        typedef plugin* (* call_get_plugin)();
#endif

        plugin_factory(void);
        ~plugin_factory(void);

        void load_plugins();

        typedef std::vector<plugin*> plugin_list;
        typedef std::vector<void*> handle_list;

        boost::mutex mutex_;
        plugin_list loaded_plugins_;
        handle_list loaded_handles_;
        static plugin_factory instance_;

    public:

        template <typename T>
        class destroy_deleter {
        public: void operator()(T* t) {
                t->destroy();
            }
        };

        template <typename T>
        class null_deleter {
        public: void operator()(T* t) {
            }
        };

        template <class T, class D> std::vector<boost::shared_ptr<T>> query_interface(const char* function_name){
            boost::mutex::scoped_lock lock(mutex_);

#if defined(WIN32)
            typedef T* (__stdcall* plugin_interface_call)();
#else if defined(UNIX)
            typedef T* (* plugin_interface_call)();
#endif

            std::vector<boost::shared_ptr<T>> plugins;
            handle_list& all_handles = instance().loaded_handles;

            typedef handle_list::iterator iterator;
            iterator current_handle = all_handles.begin();
            while (current_handle != all_handles.end()){
                plugin_interface_call function_ptr =

#if defined(WIN32)
                    (plugin_interface_call) GetProcAddress((HMODULE)(*current_handle), function_name);
#else if defined(UNIX)
                    (plugin_interface_call) dlsym(*current_handle, function_name);
#endif

                if(function_ptr) {
                    T* result = function_ptr();
                    if (result) {
                        plugins.push_back(boost::shared_ptr<T>(result, D()));
                    }

                }

                current_handle++;
            }

            return plugins;
        }
    };
}

#endif

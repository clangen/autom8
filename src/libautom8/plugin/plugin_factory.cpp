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

#include <autom8/plugin/plugin_factory.hpp>
#include <boost/filesystem.hpp>

using namespace autom8;

plugin_factory plugin_factory::instance_;

#if defined(WIN32)
typedef boost::filesystem::wpath path_type;
typedef boost::filesystem::wdirectory_iterator directory_iterator_type;
typedef std::wstring path_string;
#define UTF(x) L ## x

path_type get_plugin_path() {
    return path_type(UTF("c:\\autom8\\plugins\\"));
}
#elif defined(UNIX)
typedef boost::filesystem::path path_type;
typedef boost::filesystem::directory_iterator directory_iterator_type;
typedef std::string path_string;
#define UTF(x) x

path_type get_plugin_path() {

}
#endif

plugin_factory::plugin_factory() {
    load_plugins();
}

plugin_factory::~plugin_factory(void) {
    // will call IPlugin deleters, which may call plugin::destroy()
    loaded_plugins_.clear();

    // Unload dynamic libraries
    typedef handle_list::iterator iterator;
    iterator it = loaded_handles_.begin();
    for( ; it != loaded_handles_.end() ; ) {
#if defined(WIN32)
        FreeLibrary((HMODULE)(*it));
#elif defined (UNIX)
        dlclose(*it);
#endif
    }

    loaded_handles_.clear();
}

void plugin_factory::load_plugins(){
    boost::mutex::scoped_lock lock(mutex_);

    path_type path = get_plugin_path();

    try {
        directory_iterator_type end_file;
        directory_iterator_type current_file(path);
        for( ; current_file != end_file; ++current_file) {
            if(boost::filesystem::is_regular(current_file->status())) {
                // This is a file
                path_string file_name(current_file->path().string());

#if defined(WIN32)
                    if(file_name.substr(file_name.size() - 4) == UTF(".dll")) {
                        HMODULE handle = LoadLibrary(file_name.c_str());
                        if(handle != NULL) {
                            call_get_plugin function_ptr = (call_get_plugin) GetProcAddress(handle, "get_plugin");
                            if(function_ptr) {
                                loaded_plugins_.push_back(function_ptr());
                                loaded_handles_.push_back(handle);
                            }
                            else {
                                FreeLibrary(handle);
                            }
                        }
                    }
#elif defined(UNIX)
                    if(file_name.substr(file_name.size() - 3) == UTF(".so")){
                        void* handle = dlopen(file_name.c_str(), RTLD_LAZ);
                        if(handle != -1) {
                            call_get_plugin function_ptr = (call_get_plugin)dlsym(handle, "get_plugin");
                            if(function_ptr) {
                                loaded_plugins_.push_back(function_ptr());
                                loaded_handles_.push_back(handle);
                            }
                            else {
                                dclose(handle);
                            }
                        }
                    }
#endif
            }
        }
    }
    catch(...) {
    }
}



#ifndef V8PP_CONFIG_HPP_INCLUDED
#define V8PP_CONFIG_HPP_INCLUDED

/// v8pp library version
#define V8PP_VERSION "@PROJECT_VERSION@"

/// v8::Isolate data slot number, used in v8pp for shared data
//#if !defined(V8PP_ISOLATE_DATA_SLOT)
//#define V8PP_ISOLATE_DATA_SLOT @V8PP_ISOLATE_DATA_SLOT@
//#endif

/// v8pp plugin initialization procedure name
#if !defined(V8PP_PLUGIN_INIT_PROC_NAME)
#define V8PP_PLUGIN_INIT_PROC_NAME @V8PP_PLUGIN_INIT_PROC_NAME@
#endif

/// v8pp plugin filename suffix
#if !defined(V8PP_PLUGIN_SUFFIX)
#define V8PP_PLUGIN_SUFFIX "@V8PP_PLUGIN_SUFFIX@"
#endif

#if defined(_MSC_VER)
#define V8PP_EXPORT __declspec(dllexport)
#define V8PP_IMPORT __declspec(dllimport)
#elif __GNUC__ >= 4
#define V8PP_EXPORT __attribute__((__visibility__("default")))
#define V8PP_IMPORT V8PP_EXPORT
#else
#define V8PP_EXPORT
#define V8PP_IMPORT
#endif

#define V8PP_PLUGIN_INIT(isolate) extern "C" V8PP_EXPORT \
v8::Local<v8::Value> V8PP_PLUGIN_INIT_PROC_NAME(isolate)

//#ifndef V8PP_HEADER_ONLY
//#define V8PP_HEADER_ONLY @V8PP_HEADER_ONLY@
//#endif

#if V8PP_HEADER_ONLY
#define V8PP_IMPL inline
#else
#define V8PP_IMPL
#endif

#define V8PP_STRINGIZE(s)  V8PP_STRINGIZE0(s)
#define V8PP_STRINGIZE0(s) #s

#endif // V8PP_CONFIG_HPP_INCLUDED

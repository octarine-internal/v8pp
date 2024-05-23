#include "v8pp/context.hpp"
#include "v8pp/config.hpp"
#include "v8pp/convert.hpp"
#include "v8pp/function.hpp"
#include "v8pp/module.hpp"
#include "v8pp/class.hpp"
#include "v8pp/throw_ex.hpp"

#include <fstream>
#include <utility>

namespace v8pp {

struct context::dynamic_module
{
	void* handle;
	v8::Global<v8::Value> exports;
};

struct array_buffer_allocator : v8::ArrayBuffer::Allocator
{
	void* Allocate(size_t length)
	{
		return calloc(length, 1);
	}
	void* AllocateUninitialized(size_t length)
	{
		return malloc(length);
	}
	void Free(void* data, size_t length)
	{
		free(data);
		(void)length;
	}
};
static array_buffer_allocator array_buffer_allocator_;

v8::Isolate* context::create_isolate(v8::ArrayBuffer::Allocator* allocator)
{
	v8::Isolate::CreateParams create_params;
	create_params.array_buffer_allocator = allocator ? allocator : &array_buffer_allocator_;

	return v8::Isolate::New(create_params);
}

context::context(v8::Isolate* isolate, v8::ArrayBuffer::Allocator* allocator,
		bool add_default_global_methods, bool enter_context,
		v8::Local<v8::ObjectTemplate> global)
	: own_isolate_(isolate == nullptr)
	, enter_context_(enter_context)
	, isolate_(isolate ? isolate : create_isolate(allocator))
{
	if (own_isolate_)
	{
		isolate_->Enter();
	}

	v8::HandleScope scope(isolate_);

	if (global.IsEmpty())
	{
		global = v8::ObjectTemplate::New(isolate_);
	}

	v8::Local<v8::Context> impl = v8::Context::New(isolate_, nullptr, global);
	if (enter_context_)
	{
		impl->Enter();
	}
	impl_.Reset(isolate_, impl);
}

context::context(context&& src) noexcept
	: own_isolate_(std::exchange(src.own_isolate_, false))
	, enter_context_(std::exchange(src.enter_context_, false))
	, isolate_(std::exchange(src.isolate_, nullptr))
	, impl_(std::move(src.impl_))
	, modules_(std::move(src.modules_))
	, lib_path_(std::move(src.lib_path_))
{
}

context& context::operator=(context&& src) noexcept
{
	if (&src != this)
	{
		destroy();

		own_isolate_ = std::exchange(src.own_isolate_, false);
		enter_context_ = std::exchange(src.enter_context_, false);
		isolate_ = std::exchange(src.isolate_, nullptr);
		impl_ = std::move(src.impl_);
		modules_ = std::move(src.modules_);
		lib_path_ = std::move(src.lib_path_);
	}
	return *this;
}

context::~context()
{
	destroy();
}

void context::destroy()
{
	if (isolate_ == nullptr && impl_.IsEmpty())
	{
		// moved out state
		return;
	}

	// remove all class singletons and external data before modules unload
	cleanup(isolate_);

	if (enter_context_)
	{
		to_local(isolate_, impl_)->Exit();
	}
	impl_.Reset();

	if (own_isolate_)
	{
		isolate_->Exit();
		isolate_->Dispose();
	}
	isolate_ = nullptr;
}

context& context::value(std::string_view name, v8::Local<v8::Value> value)
{
	v8::HandleScope scope(isolate_);
	global()->Set(isolate_->GetCurrentContext(), to_v8(isolate_, name), value).FromJust();
	return *this;
}

context& context::module(std::string_view name, v8pp::module& m)
{
	return value(name, m.new_instance());
}

} // namespace v8pp

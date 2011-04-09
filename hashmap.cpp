#include <cstdlib>
#include <node.h>
#include <node_buffer.h>
#include <ext/hash_map>

using namespace v8;
using namespace node;
using namespace __gnu_cxx;

namespace __gnu_cxx
{
    template<> struct hash<Value*>
    {
        size_t operator()(const Value* x) const
        {
            if(x->IsObject())
                return (size_t)((Object*)x)->GetIdentityHash();
            if(x->IsNumber())
                return (size_t)((Number*)x)->Value();
            if(x->IsDate())
                return (size_t)((Date*)x)->NumberValue();
            if(x->IsString())
                return hash< const char* >()( *String::AsciiValue(Handle<Value>(const_cast<Value*>(x))) );
            if(x->IsTrue())
                return 1;
            return 0;
        }
    };
}

struct ValueEqual
{
    bool operator()(Value* v1, Value* v2) const
    {
        return v1->Equals(Handle<Value>(v2));
    }
};

typedef hash_map<Value*, Value*, hash<Value*>, ValueEqual> ValueHash;

class HashMap : public ObjectWrap {
    //HashMap constructor
    static Persistent<FunctionTemplate> constructor_template;
    //internal hashmap
    ValueHash m_hashmap;

public:
    HashMap(){
        m_hashmap = ValueHash();
    }

    static void Initialize(Handle<Object> target) {
        HandleScope scope;

        Local<FunctionTemplate> t = FunctionTemplate::New(New);
        constructor_template = Persistent<FunctionTemplate>::New(t);
        constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
        constructor_template->SetClassName(String::NewSymbol("HashMap"));

        NODE_SET_PROTOTYPE_METHOD(constructor_template, "set", Set);
        NODE_SET_PROTOTYPE_METHOD(constructor_template, "get", Get);
        NODE_SET_PROTOTYPE_METHOD(constructor_template, "remove", Remove);
        NODE_SET_PROTOTYPE_METHOD(constructor_template, "each", Each);
        NODE_SET_PROTOTYPE_METHOD(constructor_template, "empty", Empty);
        target->Set(String::NewSymbol("HashMap"), constructor_template->GetFunction());
    }

    static Handle<Value> New(const Arguments &args) {
        HandleScope scope;

        HashMap* hashmap = new HashMap();
        hashmap->Wrap(args.This());

        return args.This();
    }

    static Handle<Value> Set(const Arguments &args){
        HashMap* hashmap = ObjectWrap::Unwrap<HashMap>(args.This());

        Persistent<Value> key = Persistent<Value>::New(args[0]);
        Persistent<Value> value = Persistent<Value>::New(args[1]);

        hashmap->set(key, value);

        return args.This();
    }

    static Handle<Value> Get(const Arguments &args){
        HashMap* hashmap = ObjectWrap::Unwrap<HashMap>(args.This());

        return hashmap->get(args[0]);
    }

    static Handle<Value> Remove(const Arguments& args)
    {
        HashMap* hashmap = ObjectWrap::Unwrap<HashMap>(args.This());

        return hashmap->remove(args[0]);
    }

    static Handle<Value> Each(const Arguments& args)
    {
        HashMap* hashmap = ObjectWrap::Unwrap<HashMap>(args.This());

        hashmap->each(Handle<Function>::Cast(args[0]));

        return Undefined();
    }

    static Handle<Value> Empty(const Arguments& args)
    {
        HashMap* hashmap = ObjectWrap::Unwrap<HashMap>(args.This());

        return hashmap->empty();
    }

    //own methods
    void set(const Handle<Value>& key, const Handle<Value>& value){
        m_hashmap[*key] = *value;
    }

    Handle<Value> get(const Handle<Value>& key){
        return Handle<Value>(m_hashmap[*key]);
    }

    Handle<Value> remove(const Handle<Value>& key)
    {
        ValueHash::iterator i = m_hashmap.find(*key);
        Handle<Value> value = Handle<Value>(i->second);
        Persistent<Value>(i->first).Dispose();
        m_hashmap.erase(i);
        return value;
    }

    void each(Handle<Function> fn)
    {
        for(ValueHash::iterator i = m_hashmap.begin(); i != m_hashmap.end(); ++i)
        {
            Local<Value> argv[2];
            argv[0] = Local<Value>(i->first);
            argv[1] = Local<Value>(i->second);

            TryCatch try_catch;

            fn->Call(Context::GetCurrent()->Global(), 2, argv);
        }
    }

    Handle<Value> empty()
    {
        return m_hashmap.empty() ? True() : False();
    }
};

Persistent<FunctionTemplate> HashMap::constructor_template;

extern "C" void
init(Handle<Object> target){
    HandleScope scope;

    HashMap::Initialize(target);
}


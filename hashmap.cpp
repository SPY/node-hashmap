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
    HashMap()
    {
        m_hashmap = ValueHash();
    }

    ~HashMap()
    {
        FreeValues();
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
        NODE_SET_PROTOTYPE_METHOD(constructor_template, "clear", Clear);
        NODE_SET_PROTOTYPE_METHOD(constructor_template, "hasKey", HasKey);
        constructor_template->PrototypeTemplate()->SetAccessor(
                String::NewSymbol("length"),
                Length
            );
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

        hashmap->each(Handle<Function>::Cast(args[0]),
                      args.Length() > 1 ?
                        Handle<Object>::Cast(args[1]) :
                        Context::GetCurrent()->Global(),
                      args.This());

        return Undefined();
    }

    static Handle<Value> Clear(const Arguments& args)
    {
        HashMap* hashmap = ObjectWrap::Unwrap<HashMap>(args.This());
        hashmap->clear();
        return Undefined();
    }

    static Handle<Value> HasKey(const Arguments& args)
    {
        HashMap* hashmap = ObjectWrap::Unwrap<HashMap>(args.This());
        return hashmap->hasKey(args[0]);
    }

    static Handle<Value> Length(Local<String> property, const AccessorInfo &info)
    {
        HashMap* hashmap = ObjectWrap::Unwrap<HashMap>(info.This());
        return hashmap->length();
    }

    //own methods
    void set(const Handle<Value>& key, const Handle<Value>& value){
        ValueHash::iterator i = m_hashmap.find(*key);
        if(i == m_hashmap.end())
            m_hashmap[*key] = *value;
        else
        {
            if(!i->second->Equals(value))
            {
                Persistent<Value>(i->second).Dispose();
                m_hashmap[*key] = *value;
            }
        }
    }

    Handle<Value> get(const Handle<Value>& key){
        ValueHash::iterator i = m_hashmap.find(*key);
        return (i == m_hashmap.end()) ? Local<Value>(*Undefined()) : Handle<Value>(i->second);
    }

    Handle<Value> remove(const Handle<Value>& key)
    {
        ValueHash::iterator i = m_hashmap.find(*key);
        if(i == m_hashmap.end())
            return Undefined();
        Handle<Value> value = Handle<Value>(i->second);
        Persistent<Value>(i->first).Dispose();
        m_hashmap.erase(i);
        return value;
    }

    void each(Handle<Function> fn, Handle<Object> scope, Handle<Object> This)
    {
        for(ValueHash::iterator i = m_hashmap.begin(); i != m_hashmap.end(); ++i)
        {
            Local<Value> argv[3];
            argv[0] = Local<Value>(i->first);
            argv[1] = Local<Value>(i->second);
            argv[2] = Local<Value>(*This);

            //TryCatch try_catch;

            fn->Call(scope, 3, argv);

        }
    }

    void clear()
    {
        FreeValues();
        m_hashmap.clear();
    }

    Handle<Value> hasKey(const Handle<Value>& key)
    {
        ValueHash::iterator i = m_hashmap.find(*key);
        return i == m_hashmap.end() ? False() : True();
    }

    Handle<Integer> length()
    {
        return Integer::NewFromUnsigned(m_hashmap.size());
    }

    private:
    void FreeValues()
    {
        for(ValueHash::iterator i = m_hashmap.begin(); i != m_hashmap.end(); ++i)
        {
            Persistent<Value>(i->first).Dispose();
            Persistent<Value>(i->second).Dispose();
        }
    }
};

Persistent<FunctionTemplate> HashMap::constructor_template;

extern "C" void
init(Handle<Object> target){
    HandleScope scope;

    HashMap::Initialize(target);
}


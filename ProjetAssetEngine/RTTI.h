#pragma once

#include <string>

#include "Factory.h"

class Archive; 

class RTTI {
public:
    virtual ~RTTI() = default; 

    [[nodiscard]] virtual const std::string& GetClassName() const noexcept = 0;

    virtual void Process(Archive& ar) = 0;
};



#define DECLARE_RTTI(ClassName) \
public: \
    [[nodiscard]]  const std::string& GetClassName() const noexcept override { \
        static const std::string name = #ClassName; \
        return name; \
    } \
    [[nodiscard]] static RTTI* CreateInstance() { \
        return new ClassName(); \
    }

#define REGISTER_RTTI(ClassName) \
    namespace { \
        struct ClassName##_Register { \
            ClassName##_Register() { \
                Factory::Register(#ClassName, &ClassName::CreateInstance); \
            } \
        }; \
        static ClassName##_Register global_##ClassName##_Register; \
    }

#define SAFE_CAST(T, obj) dynamic_cast<T*>(obj)
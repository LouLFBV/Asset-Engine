#include "Archive.h"
#include "Factory.h"
#include "RTTI.h"

void Archive::Process(const char* Key, RTTI*& Data) {
    std::string className;
    std::string classKey = Key ? std::string(Key) + "_class" : "class";

    if (IsWriting()) {
        className = (Data != nullptr) ? Data->GetClassName() : "Null";
        Process(classKey.c_str(), className);

        if (Data != nullptr) {
            Data->Process(*this);
        }
    }
    else {
        Process(classKey.c_str(), className);

        if (className != "Null") {
            Data = Factory::Create(className);
            if (Data != nullptr) {
                Data->Process(*this);
            }
        }
        else {
            Data = nullptr;
        }
    }
}
#pragma once

#include <stdexcept>

struct IStreamFILE{
     static FILE *new_from_stream(std::istream &stream, const char *mode){
        throw std::runtime_error("unimplemented");
        return nullptr;
     };
};
struct OStreamFILE{
    static FILE *new_from_stream(std::ostream &stream, const char *mode){
        throw std::runtime_error("unimplemented");
        return nullptr;
     };
};

#pragma once

class Bytes {
    public:
    char * value;
    size_t length;
    Bytes() {}
    Bytes(char * v, long l) {
        value = v;
        length = l;
    }
};
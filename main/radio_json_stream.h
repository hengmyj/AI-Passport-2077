#pragma once
#include <cstddef>

// Bounded, incremental splitter for a JSON array of objects. A completed object
// is parsed separately by cJSON; oversized entries are skipped, never truncated.
struct RadioJsonObjects {
    char object[4096]{};
    std::size_t used=0;
    unsigned depth=0;
    bool quoted=false,escaped=false,overflow=false,done=false,failed=false;
    enum Phase { Start, First, Value, Comma } phase=Start;
    bool feed(char c) {
        if(failed)return false;
        if(!depth){
            if(c==' '||c=='\r'||c=='\n'||c=='\t')return false;
            if(done){failed=true;return false;}
            if(phase==Start){if(c=='[')phase=First;else failed=true;return false;}
            if(c==']'&&(phase==First||phase==Comma)){done=true;return false;}
            if(phase==Comma){if(c==',')phase=Value;else failed=true;return false;}
            if(c!='{'){failed=true;return false;}
            used=0;overflow=false;depth=1;quoted=false;escaped=false;
        }else if(quoted){
            if(escaped)escaped=false;
            else if(c=='\\')escaped=true;
            else if(c=='"')quoted=false;
        }else if(c=='"')quoted=true;
        else if(c=='{')++depth;
        else if(c=='}')--depth;
        if(used+1<sizeof(object))object[used++]=c;else overflow=true;
        if(!depth){object[used]=0;phase=Comma;return !overflow;}
        return false;
    }
};

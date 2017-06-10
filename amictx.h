#if ! defined(AMICTX_H)
#define AMICTX_H

///////////////////////////////////////////////////////////////////////////////
extern "C" {
///////////////////////////////////////////////////////////////////////////////

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/copper.h>
#include <graphics/videocontrol.h>
#include <intuition/intuition.h>
#include <intuition/preferences.h>
#include <hardware/custom.h>
#include <libraries/dos.h>
//
#include <clib/exec_protos.h>           
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

extern struct Custom custom;

///////////////////////////////////////////////////////////////////////////////
} // extern "C"
///////////////////////////////////////////////////////////////////////////////

typedef unsigned short u16;

///////////////////////////////////////////////////////////////////////////////

struct rgb {
    rgb(float r, float g, float b) : _r(r), _g(g), _b(b) {}
    u16 RGB12() const { 
        return (u16(_r*15.0f)<<8)|(u16(_g*15.0f)<<4)|(u16(_b*15.0f));
    }
    static rgb lerp(const rgb& c1, const rgb& c2, float i) {
        float ii = 1.0f-i;
        float r = (c1._r*ii)+(c2._r*i);
        float g = (c1._g*ii)+(c2._g*i);
        float b = (c1._b*ii)+(c2._b*i);
        return rgb(r,g,b);
    }
    float _r, _g, _b;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T> struct ScopedObj {

    ScopedObj()
        : _object(NULL)
    {
        _object = (T*) AllocMem(sizeof(T), MEMF_PUBLIC|MEMF_CLEAR );
        printf("_object<%p>\n", _object );
        assert(_object != NULL);
    }
    ~ScopedObj()
    {
        if(_object)
            FreeMem((APTR)_object,sizeof(T));
    }
    T* _object;
};

template <typename T> T* MakeObject() {
    return (T*) AllocMem(sizeof(T), MEMF_PUBLIC|MEMF_CLEAR );
}

///////////////////////////////////////////////////////////////////////////////

struct amilib {   

    amilib( const char* name, int version ); 
    ~amilib();

    Library* _library;
    const char* _name;
};

///////////////////////////////////////////////////////////////////////////////

struct amictx  {

    amictx();
    ~amictx();
    void flip();
    void initCopper();
    void updateCopper();
    
    ///////////////////////////////////////

    amilib _libgfx, _libgui, _liblev;

    Screen* _screen;
    MsgPort* _dispPort;
    MsgPort* _safePort;
    Window* _window;
    //RastPort* _rport;
    UCopList* _ucoplist;
    ScreenBuffer* _screenbuf[2];
    ScopedObj<BitMap> _bitmap[2];

    bool _safeToWrite, _safeToChange;
    int _dbIDX;
};

///////////////////////////////////////////////////////////////////////////////
#endif
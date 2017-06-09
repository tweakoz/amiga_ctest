extern "C" {
///////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////
#include <clib/exec_protos.h>           
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
///////////////////////////////////////////////////////////
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
extern struct Custom custom;
///////////////////////////////////////////////////////////
}

typedef unsigned short u16;

///////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////

struct amilib {   

    amilib( const char* name, int version )
        : _library(NULL)
    {
        _library = OpenLibrary(name,version);
    };
 
    ~amilib()
    {
        if( _library )
        {
            CloseLibrary(_library);
            _library = NULL;
        }
    }
    Library* _library;
};

///////////////////////////////////////////////////////////

struct amictx  {

    ///////////////////////////////////////

    amictx()
        : _libgfx("graphics.library",37)
        , _libgui("intuition.library",37)
    {
        
        ///////////////////////////////////////
        //  Prepare to explicitly request Topaz 60 as the screen font. 
        ///////////////////////////////////////

        struct TextAttr topaz60 = {
            (STRPTR)"topaz.font",
            (UWORD)TOPAZ_SIXTY, (UBYTE)0, (UBYTE)0
        };

        ///////////////////////////////////////

        _screen = OpenScreenTags( NULL,
             SA_Overscan, OSCAN_STANDARD,
             SA_Title,    "Yo",
             SA_Font,     (ULONG)&topaz60,
             //SA_Width,  640,
             //SA_Height, 480,
             //SA_Depth, 4,
             //SA_Type, SCREENHIRES,
             TAG_DONE);

        assert(_screen!=NULL);

        ///////////////////////////////////////

        const int WINWIDTH = 320; //  Width of window.  
        const int THEIGHT = _screen->Font->ta_YSize + 3;

        _window = OpenWindowTags( NULL,
             WA_CustomScreen, _screen,
             WA_Title,        "what up yo",
             WA_IDCMP,        CLOSEWINDOW,
             WA_Flags,        WINDOWDRAG|WINDOWCLOSE|INACTIVEWINDOW,
             WA_Left,         (_screen->Width-WINWIDTH)/2,
             WA_Top,          _screen->Height-THEIGHT,
             WA_Height,       THEIGHT,
             WA_Width,        WINWIDTH,
             TAG_DONE);

        assert(_window!=NULL);

        _ucoplist = MakeObject<UCopList>();

        initCopper();
    }

    ///////////////////////////////////////

    ~amictx() {

        if(_window)
            CloseWindow(_window);

        if(_screen)
            CloseScreen(_screen);
    }

    ///////////////////////////////////////

    void initCopper();

    ///////////////////////////////////////

    amilib _libgfx;
    amilib _libgui;
    Screen* _screen;
    Window* _window;
    UCopList* _ucoplist;
};

///////////////////////////////////////////////////////////

int main( int argc, char** argv ){

    amictx ctx;

    ///////////////////////////////////////////////////
    //  Wait until the user clicks in the close gadget.  
    ///////////////////////////////////////////////////

    Wait(1<<ctx._window->UserPort->mp_SigBit);

    bool done = false;
    while(false==done) {
        IntuiMessage* intuiMessage = (IntuiMessage *) GetMsg(ctx._window->UserPort);
        if(intuiMessage)
            ReplyMsg((Message *)intuiMessage);
        else
            done = true;
    }
    
    ///////////////////////////////////////////////////

    ViewPort* viewPort = ViewPortAddress(ctx._window);
    if (NULL != viewPort->UCopIns) {
        FreeVPortCopLists(viewPort);
        RemakeDisplay();
    }

    ///////////////////////////////////////////////////

    return 0;
}

///////////////////////////////////////////////////////////
// loadCopper() -- creates a Copper list program and adds it to the system
///////////////////////////////////////////////////////////

void amictx::initCopper() {

    size_t numc = 64;

    rgb ctop(0,0,0), cmid(0,.4,.6), cbot(1,.4,.2);

    CINIT(_ucoplist, numc);

    int segh = _screen->Height/(numc);

    for( int i=0; i<40; i++ )
    {
        float fi = float(i)/float(40);
        rgb C = rgb::lerp(ctop,cmid,fi);
        u16 RGB12 = C.RGB12();
        if(i<3) RGB12 = 0xf00;
        CWAIT(_ucoplist, (i*segh), 0);
        CMOVE(_ucoplist, custom.color[0], RGB12);
    }
    for( int i=40; i<64; i++ )
    {
        float fi = float(i-40)/float(24);
        rgb C = rgb::lerp(cmid,cbot,fi);
        u16 RGB12 = C.RGB12();
        CWAIT(_ucoplist, (i*segh), 0);
        CMOVE(_ucoplist, custom.color[0], RGB12);
    }
    
    CEND(_ucoplist); 

    ViewPort* viewPort = ViewPortAddress(_window);     
    
    Forbid(); //  begin critical section
    {
        viewPort->UCopIns=_ucoplist;
    }
    Permit(); //  end critical section 

    struct TagItem  uCopTags[] = {
        { VTAG_USERCLIP_SET, NULL }, // enable copperlist clipping
        { VTAG_END_CM, NULL }
    };

    VideoControl( viewPort->ColorMap, uCopTags );

    RethinkDisplay(); //  Display the new Copper list. 

}

///////////////////////////////////////////////////////////


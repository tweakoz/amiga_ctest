#include "amictx.h"

///////////////////////////////////////////////////////////////////////////////

amictx::amictx()
    : _libgfx("graphics.library",39)
    , _libgui("intuition.library",39)
    , _liblev("lowlevel.library",39)
    , _screen(NULL)
    , _window(NULL)
    , _dispPort(NULL)
    , _safePort(NULL)
    , _safeToWrite(true)
    , _safeToChange(true)
    , _dbIDX(1)
{
    ///////////////////////////////////////
    //  Prepare to explicitly request Topaz 60 as the screen font. 
    ///////////////////////////////////////

    struct TextAttr topaz60 = {
        (STRPTR)"topaz.font",
        (UWORD)TOPAZ_SIXTY, (UBYTE)0, (UBYTE)0
    };

    ///////////////////////////////////////

    const int width = 640;
    const int height = 200;
    const int depth = 4;

    ///////////////////////////////////////

    struct openmon {
        static ULONG doit(ULONG monid,int w,int h, int d) {
            return BestModeID(   BIDTAG_NominalWidth, w,
                                 BIDTAG_NominalHeight, h,
                                 BIDTAG_DesiredWidth, w,
                                 BIDTAG_DesiredHeight, h,
                                 BIDTAG_Depth, d,
                                 BIDTAG_MonitorID, monid,
                                 TAG_END);

        }
    };

    ///////////////////////////////////////
    
    /*ULONG modeId = openmon::doit(NTSC_MONITOR_ID,width,height,depth);
    if (modeId == INVALID_ID) 
        modeId = openmon::doit(PAL_MONITOR_ID,width,height,depth);
    if (modeId == INVALID_ID) 
        modeId = openmon::doit(NTSC_MONITOR_ID,width,height,depth);

    assert(modeId != INVALID_ID);*/

    ///////////////////////////////////////

    _screen = OpenScreenTags( NULL,
        SA_Depth, depth,
        //SA_DisplayID, modeId,
        SA_Width, width,
        SA_Height, height,
        SA_Type, CUSTOMSCREEN,
        SA_Overscan, OSCAN_TEXT,
        SA_Title, "yo",
        SA_ShowTitle, TRUE,
        SA_Draggable, FALSE,
        SA_Exclusive, TRUE,
        SA_AutoScroll, FALSE,
        TAG_END);

    assert(_screen!=NULL);

    ///////////////////////////////
    // init double buffers
    ///////////////////////////////

   /* _dispPort = CreateMsgPort();
    _safePort = CreateMsgPort();

    _screenbuf[0] = AllocScreenBuffer(_screen, NULL, SB_SCREEN_BITMAP);
    _screenbuf[1] = AllocScreenBuffer(_screen, NULL, 0);

    assert(_screenbuf[0] != NULL );
    assert(_screenbuf[1] != NULL );

    for( int i=0; i<2; i++ ) {
        _screenbuf[i]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = _dispPort;
        _screenbuf[i]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = _safePort;
    }*/

    ///////////////////////////////////////

    _window = OpenWindowTags( NULL,
       WA_Left, 0,
       WA_Top, 0,
       WA_Width, width,
       WA_Height, height,
       WA_Title, NULL,
       WA_CustomScreen, (ULONG)_screen,
       WA_Backdrop, TRUE,
       WA_Borderless, TRUE,
       WA_DragBar, FALSE,
       WA_Activate, TRUE,
       WA_SmartRefresh, TRUE,
       WA_NoCareRefresh, TRUE,
       WA_ReportMouse, TRUE,
       WA_RMBTrap, TRUE,
       WA_IDCMP, IDCMP_RAWKEY
               | IDCMP_MOUSEMOVE
               | IDCMP_MOUSEBUTTONS
               | IDCMP_ACTIVEWINDOW
               | IDCMP_INACTIVEWINDOW,
       TAG_END);



    assert(_window!=NULL);

    ///////////////////////////////
    // init copper list
    ///////////////////////////////

    initCopper();

    ///////////////////////////////
    struct freePlanes{
    static VOID doit(ScopedObj<BitMap>& bmapw, LONG depth, LONG width, LONG height)
    {
        BitMap* bitmap = bmapw._object;

        for(int plane_num = 0; plane_num < depth; plane_num++)
        {
            if (bitmap->Planes[plane_num] != NULL)
                FreeRaster(bitmap->Planes[plane_num], width, height);
        }
    }};
    struct setupPlanes{
    static LONG doit(ScopedObj<BitMap>& bmapw, LONG depth, LONG width, LONG height)
    {
        BitMap* bitmap = bmapw._object;

        for( int plane_num = 0; plane_num < depth; plane_num++ )
            {
            bitmap->Planes[plane_num] = (PLANEPTR)AllocRaster(width, height);
            if (bitmap->Planes[plane_num] != NULL )
                BltClear(bitmap->Planes[plane_num], (width / 8) * height, 1);
            else
                {
                freePlanes::doit(bmapw, depth, width, height);
                return(NULL);
                }
            }
        return(TRUE);
    }};

    InitBitMap(_bitmap[0]._object, depth, width, height);
    InitBitMap(_bitmap[1]._object, depth, width, height);

    setupPlanes::doit(_bitmap[0],depth,width,height);
    setupPlanes::doit(_bitmap[1],depth,width,height);
}

///////////////////////////////////////////////////////////////////////////////

amictx::~amictx() {

    printf( "closing amictx<%p>\n", this );

    WaitBlit();

    /*FreeScreenBuffer(_screen, _screenbuf[0]);
    FreeScreenBuffer(_screen, _screenbuf[1]);

    if (!_safeToWrite) {
        while(!GetMsg(_safePort)) {
            Wait(1l<<(_safePort->mp_SigBit));
        }
    }
    DeleteMsgPort(_safePort);

    if (!_safeToChange) {
        while(!GetMsg(_dispPort)) {
            Wait(1l<<(_dispPort->mp_SigBit));
        }
    }
    DeleteMsgPort(_dispPort);*/

    if(_window)
        CloseWindow(_window);

    if(_screen)
        CloseScreen(_screen);
}

///////////////////////////////////////////////////////////
// loadCopper() -- creates a Copper list program and adds it to the system
///////////////////////////////////////////////////////////

void amictx::initCopper() {

    _ucoplist = MakeObject<UCopList>();
    updateCopper();
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

void amictx::updateCopper() {

    size_t numc = 64;

    rgb ctop(0,0,0), cmid(0,.4,.6), cbot(1,.4,.2);

    CINIT(_ucoplist, numc);

    int segh = _screen->Height/(numc);

    for( int i=0; i<40; i++ )
    {
        float fi = float(i)/float(40);
        rgb C = rgb::lerp(ctop,cmid,fi);
        u16 RGB12 = C.RGB12();
        //if(i<3) RGB12 = 0xf00;
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

}
///////////////////////////////////////////////////////////////////////////////

void amictx::flip()
{
    // Wait for safe to draw.
   // while(!GetMsg(_dispPort)) {
   //     Wait(1l<<(_dispPort->mp_SigBit));
    //}


    while(!GetMsg(_safePort)) {
        Wait(1l<<(_safePort->mp_SigBit));
    }

    assert( (_dbIDX>=0) && (_dbIDX<2) );

    while( ChangeScreenBuffer(_screen, _screenbuf[_dbIDX]) == 0 )
    {

    }

    _dbIDX = _dbIDX ^ 1;

    printf( "dbIDX<%d>\n", _dbIDX );

}

///////////////////////////////////////////////////////////////////////////////

amilib::amilib( const char* name, int version )
    : _library(NULL)
    , _name(name)
{
    _library = OpenLibrary(name,version);
    printf( "lib<%s> opened at<%p>\n", name, _library );
};

///////////////////////////////////////////////////////////////////////////////

amilib::~amilib()
{
    if( _library )
    {
        CloseLibrary(_library);
        printf( "lib<%s> closed from<%p>\n", _name, _library );
        _library = NULL;
    }
}
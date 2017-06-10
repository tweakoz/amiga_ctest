///////////////////////////////////////////////////////////////////////////////
#include "amictx.h"
///////////////////////////////////////////////////////////////////////////////

static unsigned long x=123456789, y=362436069, z=521288629;

unsigned short fastrand(void) {          //period 2^96-1
unsigned long t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

   t = x;
   x = y;
   y = z;
   z = t ^ x ^ y;

  return short(z&0xffff);
}

int main( int argc, char** argv ){

    amictx ctx;

    ///////////////////////////////////////////////////
    //  Wait until the user clicks in the close gadget.  
    ///////////////////////////////////////////////////

    //Wait(1<<ctx._window->UserPort->mp_SigBit);

    /*bool done = false;
    while(false==done) {
        IntuiMessage* intuiMessage = (IntuiMessage *) GetMsg(ctx._window->UserPort);
        if(intuiMessage)
            ReplyMsg((Message *)intuiMessage);
        else
            done = true;
    }*/

    RastPort& SR = ctx._screen->RastPort;

    RastPort R[16];
    for( int i=0; i<16; i++ )
    {
        R[i] = (SR);
        SetAPen(R+i, i);
    }

    for( short i=0; i < 30000; i++ )
    {
        short dbIDX = i&1;

        BitMap* bm = ctx._bitmap[dbIDX]._object;

        ctx._screen->ViewPort.RasInfo->BitMap = bm;

        for( short i=0; i<16; i++ )
            R[i].BitMap = bm;

        //SetRast(R, 0); // clear screen

        UBYTE* plA = bm->Planes[0];
        UBYTE* plB = bm->Planes[1];
        UBYTE* plC = bm->Planes[2];
        UBYTE* plD = bm->Planes[3];

        short pen = rand()%16;

        bool bA = pen&1;
        bool bB = pen&2;
        bool bC = pen&4;
        bool bD = pen&8;

        for( short j=0; j<100; j++ )
        {
            short x1 = fastrand()%320;
            short y1 = fastrand()%200;

            short rowptr = y1 * bm->BytesPerRow;            
            short byt = x1/8;
            UBYTE orb = 1<<(x1&7);

            short bidx = rowptr+byt;

            /////////////////////
            // update bitplanes
            /////////////////////

            if( bA ) {
                UBYTE* pixa = (plA+bidx);
                (*pixa) |= orb;
            }
            if( bB ) {
                UBYTE* pixa = (plB+bidx);
                (*pixa) |= orb;
            }
            if( bC ) {
                UBYTE* pixa = (plC+bidx);
                (*pixa) |= orb;
            }
            if( bD ) {
                UBYTE* pixa = (plD+bidx);
                (*pixa) |= orb;
            }

            /////////////////////
        }

        if( i==0 )
          MakeScreen(ctx._screen); // update screenVP's bitmap

        //ctx.updateCopper();
        if( i==0 )
            RethinkDisplay(); // 

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


///////////////////////////////////////////////////////////////////////////////
#include "amictx.h"
///////////////////////////////////////////////////////////////////////////////

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

    RastPort* R = & (ctx._screen->RastPort);

    for( int i=0; i < 300; i++ )
    {

        int dbIDX = i&1;

        ctx._screen->RastPort.BitMap = ctx._bitmap[dbIDX]._object;
        ctx._screen->ViewPort.RasInfo->BitMap = ctx._bitmap[dbIDX]._object;

        //SetRast(R, 0); // clear screen

        for( int j=0; j<30; j++ )
        {
            int x1 = rand()%320;
            int y1 = rand()%200;
            int pen = rand()%16;
            SetAPen(R, pen);
            WritePixel(R, x1, y1);
        }

        MakeScreen(ctx._screen); // Tell intuition to do its stuff

        //ctx.updateCopper();
        RethinkDisplay();

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


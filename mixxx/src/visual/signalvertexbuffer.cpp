/***************************************************************************
                          signalvertexbuffer.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen and Kenny 
                                       Erleben
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "signalvertexbuffer.h"
#include "../reader.h"
#include "../readerextract.h"
#include "../readerevent.h"
#include "fastvertexarray.h"

/**
 * Default Constructor.
 */
SignalVertexBuffer::SignalVertexBuffer(Reader *_reader, FastVertexArray *_vertex)
{
    installEventFilter(this);

    vertex = _vertex;
    reader = _reader;
    //*************************
    readerExtract;// = reader->getSoundBuffer();

    len = vertex->getSize();
    displayLen = len/2; // QUICK AND DIRTY HACK!
    buffer = vertex->getStartPtr();

    qDebug("len: %i, displayLen: %i, READCHUNKSIZE: %i, chunkSize: %i",len,displayLen,READCHUNKSIZE,vertex->getChunkSize());
    
    // Reset buffer
    GLfloat *p = buffer;
    for (int i=0; i<len; i++)
    {
        *p++ = (float)i;
        *p++ = 0.; //(float)cos((i/(1.0f*len))*6.28*5+1.5707963267);
        *p++ = 0.;
    }

    installEventFilter(this);   
}

/**
 * Deconstructor.
 */
SignalVertexBuffer::~SignalVertexBuffer()
{
};

bool SignalVertexBuffer::eventFilter(QObject *o, QEvent *e)
{
    //qDebug("SignalVertexBuffer: event");

    // If a user events are received, update containers
    if (e->type() == (QEvent::Type)10002)
    {
        ReaderEvent *re = (ReaderEvent *)e;

        update(re->pos(), re->len());
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(o,e);
    }
    return TRUE;
}

/**
 * Updates buffer.
 *
 * Input: playpos - Play position relative to the vertex buffer
 *
 */ 
void SignalVertexBuffer::update(int pos, int len)
{
//    soundbuffer->lockBuffer();
    
    CSAMPLE *source = &readerExtract->getBasePtr()[pos];

    GLfloat resampleFactor = (GLfloat)READCHUNKSIZE/(GLfloat)vertex->getChunkSize();
    GLfloat *dest = &buffer[(int)(pos/resampleFactor)*3];

    for (int i=0; i<READCHUNKSIZE; i+=resampleFactor)
    {
        GLfloat val = 0;
        for (int j=i; j<i+resampleFactor; j++)
            val += source[j]*(1./32768.);

        *dest++;
        *dest++ = val/resampleFactor;
        *dest++;
    }
    
//    soundbuffer->unlockBuffer();
}

/**
 * Retrieve Vertex Array Pointer. The buffer can be wrapped, and
 * thus pointers to two arrays are returned as bufInfo types.
 *
 * @return          A bufInfo struct containing a pointer to the samples,
 *                  and the number of samples.
 */
bufInfo SignalVertexBuffer::getVertexArray()
{
    //*******************************
    int playpos;// = reader->getPlaypos(DISPLAYRATE);
    int pos = playpos-(displayLen/2);
    while (pos<0)
        pos += len;

    bufInfo i;
    i.p1   = &buffer[pos*3];
    i.len1 = min(pos+displayLen,len)-pos;
    i.p2   = buffer;
    i.len2 = displayLen-i.len1;

    //qDebug("Total pos %i",i.len1+i.len2);
    //std::cout << "pos " << pos << ", len1 " << i.len1 << ", len2 " << i.len2 << ", displayLen " << displayLen << "\n";

    return i;
};

/**
 * Get Total Number of Samples in buffer.
 *
 * @return    The number of samples in buffer.
 */
int SignalVertexBuffer::getBufferLength()
{
    return len;
};

/**
 * Get Total Number of display samples.
 *
 * @return    The number of samples to display.
 */
int SignalVertexBuffer::getDisplayLength()
{
    return displayLen;
};


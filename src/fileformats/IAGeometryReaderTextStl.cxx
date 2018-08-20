//
//  IAFmtObjStl.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAGeometryReaderTextStl.h"

#include "Iota.h"
#include "geometry/IAMesh.h"

#include <FL/fl_utf8.h>

#include <fcntl.h>
#ifdef _WIN32
# include <io.h>
#else
# include <unistd.h>
#endif


/**
 * Create a file reader for the indicated file.
 * \return 0 if the format is not STL
 */
std::shared_ptr<IAGeometryReader> IAGeometryReaderTextStl::findReaderFor(const char *filename)
{
    int f = fl_open(filename, O_RDONLY);
    if (f==-1) {
        Iota.setError("STL Geometry reader", Error::CantOpenFile_STR_BSD, filename);
        return nullptr;
    }

    uint8_t data[5];
    size_t n = ::read(f, data, 5);
    ::close(f);

    if (n<5)  // TODO: set error
        return nullptr;

    if (strncmp((char*)data, "solid", 5)!=0)   // TODO: set error
        return nullptr;

    return std::make_shared<IAGeometryReaderTextStl>(filename);
}


/**
 * Create a reader for the indicated memory block.
 * \return 0 if the format is not STL
 */
std::shared_ptr<IAGeometryReader> IAGeometryReaderTextStl::findReaderFor(const char *name, uint8_t *data, size_t size)
{
    if (size<5)  // TODO: set error
        return nullptr;

    if (strncmp((char*)data, "solid", 5)!=0)   // TODO: set error
        return nullptr;

    return std::make_shared<IAGeometryReaderTextStl>(name, data, size);
}


/**
 * Create a file reader for reading from memory.
 */
IAGeometryReaderTextStl::IAGeometryReaderTextStl(const char *name, uint8_t *data, size_t size)
:   IAGeometryReader(name, data, size)
{
}


/**
 * Create a file reader for reading from a file.
 */
IAGeometryReaderTextStl::IAGeometryReaderTextStl(const char *filename)
:   IAGeometryReader(filename)
{
}


/**
 * Release resources.
 */
IAGeometryReaderTextStl::~IAGeometryReaderTextStl()
{
}


/**
 * Interprete the geometry data and create a mesh list.
 */
IAMesh *IAGeometryReaderTextStl::load()
{
    /*
        solid name
        facet normal ni nj nk
            outer loop
                vertex v1x v1y v1z
                vertex v2x v2y v2z
                vertex v3x v3y v3z
            endloop
        endfacet
     */

    IAMesh *msh = new IAMesh();

    // the first word must be "solid"
    getWord();
    if (!wordIs("solid")) goto fileFormatErr;
    // the rest of the line is not important
    getLine();

    for (;;) {
        double x, y, z;
        size_t p1, p2, p3;

        // check if this is the end of the facet list
        getWord();
        if ( wordIs("endsolid") )
            break;

        // if this is not the end, it must be another facet
        if (!wordIs("facet")) goto fileFormatErr;
        getWord();

        // read the facet normal and ignore it
        if (!wordIs("normal")) goto fileFormatErr;
        getDouble();
        getDouble();
        getDouble();
        getWord();

        // is this the outer loop?
        if (!wordIs("outer")) goto fileFormatErr;
        getWord();
        if (!wordIs("loop")) goto fileFormatErr;
        getWord();

        // read the first vertex
        if (!wordIs("vertex")) goto fileFormatErr;
        x = getDouble();
        y = getDouble();
        z = getDouble();
        p1 = msh->addPoint(x, y, z);
        msh->vertexList[p1]->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);

        // read the second vertex
        getWord();
        if (!wordIs("vertex")) goto fileFormatErr;
        x = getDouble();
        y = getDouble();
        z = getDouble();
        p2 = msh->addPoint(x, y, z);
        msh->vertexList[p2]->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);

        // read the third vertex
        getWord();
        if (!wordIs("vertex")) goto fileFormatErr;
        x = getDouble();
        y = getDouble();
        z = getDouble();
        p3 = msh->addPoint(x, y, z);
        msh->vertexList[p3]->pTex.set(x*0.8+0.5, -z*0.8+0.5, 0.0);

        // this should be the end of the loop
        // Some files have additional vertices here, but that is agains the
        // standard. SO for now, we error out here. We could let it slip, but
        // to support this perfectly, we would need to tesselate the emtire
        // llop then.
        getWord();
        // FIXME: word can actually be "vertex" to add more vertices to this polygon
        if (!wordIs("endloop")) goto fileFormatErr;

        // this should be the end of the facet
        getWord();
        if (!wordIs("endfacet")) goto fileFormatErr;

        // add the triangle that was generated by those vertice
        IATriangle *t = new IATriangle();
        t->pVertex[0] = msh->vertexList[p1];
        t->pVertex[1] = msh->vertexList[p2];
        t->pVertex[2] = msh->vertexList[p3];
        msh->addFace(t);
    }

    // FIXME: fixing the mesh should be done after loading *any* mesh, not just this one
    msh->validate();
    // TODO: fix seams
    // TODO: fix zero size holes
    // TODO: fix degenrate triangles
    msh->fixHoles();
    msh->validate();

    msh->clearNormals();
    msh->calculateNormals();

    return msh;

fileFormatErr:
    Iota.setError("Read Text based STL File", Error::FileContentCorrupt_STR, getName());
    delete msh;
    return nullptr;
}



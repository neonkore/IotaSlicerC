//
//  IAFramebuffer.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_FRAMEBUFFER_H
#define IA_FRAMEBUFFER_H

#include "Iota.h"

#include <FL/gl.h>
#include <FL/glu.h>

#include <memory>


class IAToolpath;


/**
 * MSWindows needs a lot of persuasion to provide some of the OpenGL calls.
 */
extern bool initializeOpenGL();


/**
 * Manage an OpenGL framebuffer object as a texture.
 *
 * Framebuffers are used to represent a single Z-layer of the overall
 * scene voxel.
 *
 * Framebuffers are filled by rendering slices, outlines, shadows, or such
 * into them. Colors can represent object ID's but can also contain true
 * color information of textured objects.
 *
 * Framebuffers can be filtered to grow or shrink objects. They can be mreged
 * to add object information, support structure information, and whatever
 * else is needed to create a vector representation.
 *
 * Framebuffer can be analysed and converted into a vector format using potrace.
 *
 * A typical GCode pipeline would render a slice through a triangle mesh.
 * A second framebuffer can be rendered defining support material.
 * In a textured model, another framebuffer can hold the shell color information
 *
 * By repeatedly vectorizing and shrinking the image in the framebuffer, GCode
 * for the outer shell is generated. The remaining graphics in the image can
 * be used to overlay a vectorized fill pattern.
 *
 * \todo allow for framebuffers without z buffer.
 */
class IAFramebuffer
{
public:
    IAFramebuffer();
    ~IAFramebuffer();
    void clear();

    void bindForRendering();
    void unbindFromRendering();

    void draw(double z);
    uint8_t *getRawImageRGB();
    uint8_t *getRawImageRGBA();
    int traceOutline(IAToolpath *toolpath, double z);
    int saveAsJpeg(const char *filename, GLubyte *imgdata=nullptr);
    int saveAsPng(const char *filename, int components, GLubyte *imgdata=nullptr);

    int width() { return pWidth; }
    int height() { return pHeight; }

protected:
    bool hasFBO();
    void activateFBO();
    void createFBO();
    void deleteFBO();

    /** Width of the framebuffer in pixles */
    int pWidth = kFramebufferSize;
    /** Height of the framebuffer in pixles */
    int pHeight = kFramebufferSize; // see Iota.h
    /** Set this flag if the OpenGL framebuffer object is created */
    bool pFramebufferCreated = false;
    /** The OpenGL framebuffer object, containing an RGBA and a depth buffer */
    GLuint pFramebuffer = 0;
    /** OpenGL reference to the RGBA color part of the framebuffer */
    GLuint pColorbuffer = 0;
    /** OpenGL reference to the depth buffer part of the framebuffer */
    GLuint pDepthbuffer = 0;

};


#endif /* IA_FRAMEBUFFER_H */



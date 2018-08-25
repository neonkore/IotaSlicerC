//
//  Iota.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

// TODO: still buggy: fix STL importer to generate only watertight models (crashes when fixing holes!)
// TODO: make STL importer bullet proof (no reads beyond end of line, for example)
// TODO: make class method names more consistent
// TODO: make high level functions automatically execute lower rank functions, if they were not run yet
// TODO: create a model class that contains meshes
//   TODO: add a positional vertex (done) and a normal (still to do) for slicing in the printer coordinate system
// TODO: create vector infills from slices
// Done (LOL)

// TODO: port to Linux

/*
 Functionality and UI ideas:

 File: things related to the app and to the Iota build file
 - New: clear the build space
 - Open...: load any file type that we understand, same as dropping a file into the 3d view
 - Save: write supported files back (\see scene format, why would we write other formats?)
 - Save As...: above, ask for filename and file type, if appropriate
 - Print...: FLTK can print OpenGL contexts, so why not?
 - Quit

 Edit:
 - Undo/Redo: this should at least undo a number og move operations, probably also unload/load files
 - Cut/Copy/Pate: is this useful?
 - Dublicate: duplicate a model
 - Delete: remove the selected mesh(es)
 - Select all/Inverse selection: Useful?

 Build:
 - Slice one or all layers
 - Analyse
 - Clean
 - Simulate GCode
 - Load/Save GCode/Layers
 - Run GCode

 Printer:
 - Setup
 - Reset
 - Send data

 Window:
 - allow multipel windows with the same scene, or with differenrt scenes?
 - Show machine link
 - Show Toolpath Element properties?
 - Anything else?

 Help:
 - Interactive calibration?
 - General Help
 - About

 Scene Format: we should probably have a format that describes what is going on'
    in the scene, which models are loaded, where they moved, what textures, so
    we can load the same setup at a later point again. This saves only links to
    files, not their content(?).
 */


#include "Iota.h"

#include "data/binaryData.h"
#include "userinterface/IAGUIMain.h"
#include "fileformats/IAFmtTexJpeg.h"
#include "fileformats/IAFmtObj3ds.h"
#include "fileformats/IAGeometryReader.h"
#include "fileformats/IAGeometryReaderBinaryStl.h"
#include "opengl/IAFramebuffer.h"
#include "toolpath/IAToolpath.h"

#include <FL/fl_ask.H>
#include <FL/fl_utf8.h>

#include <errno.h>

#ifdef _WIN32
#include <ShlObj.h>
#endif


IAIota Iota;


#define HDR "Error in Iota Slicer, %s\n\n"

/**
 * Error messages for error code from the Error enum class.
 *
 * These strings are called with the varargs
 *  1) function: a human readable location where this error occured.
 *  2) a caller provided string, often a file name
 *  3) the cause of an error as a text returned by the operating system
 */
const char *IAIota::kErrorMessage[] =
{
    // NoError
        HDR"No error.",
    // CantOpenFile_STR_BSD
        HDR"Can't open file \"%s\":\n%s",
    // UnknownFileType_STR
        HDR"Unknown and unsupported file type:\n\"%s\"",
    // FileContentCorrupt_STR
        HDR"There seems to be an error inside this file:\n\"%s\"",
	// OpenGLFeatureNotSupported_STR
		HDR"Required OpenGL graphics feature not suported:\n\"%s\"",
};


/**
 * Creat the Iota Slicer application.
 */
IAIota::IAIota()
:   pCurrentToolpath( new IAToolpath(0.0) )
{
}


/**
 * Release all resources.
 * \todo add a clear() function, so we can reset Ioat into accepting a new scene.
 */
IAIota::~IAIota()
{
    delete pMachineToolpath;
    delete pCurrentToolpath;
    delete pMesh;
    if (pErrorString)
        ::free((void*)pErrorString);
}


/**
 * Load a list of files from mass storage.
 *
 * Read geometries, textures, GCode files, or whatever the user throws at us.
 * Files are read in the order in which they appear in the list, and reading
 * stops as soon as one file reader creates an error.
 *
 * This does not delete any previously loaded data. Use clear() for that.
 *
 * This does not return an error code. Query succes by calling hadError()
 * and possibly showError().
 *
 * \todo At this point, we only know how to read STL files.
 * \todo Currently, we only support one mesh, which will be replaced by
 *       whatever we read.
 *
 * \param list one or more filenames, separated by \n
 */
void IAIota::loadAnyFile(const char *list)
{
    // loop through all entries in the list
    const char *fnStart = list, *fnEnd = list;
    for (;;) {
        fnEnd = strchr(fnStart, '\n');
        if (!fnEnd) fnEnd = fnStart+strlen(fnStart);
        if (fnEnd!=fnStart) {
            // We found a filename; now go and read that file.
            clearError();
            char *filename = (char*)calloc(1, fnEnd-fnStart+1);
            memmove(filename, fnStart, fnEnd-fnStart);
            const char *ext = fl_filename_ext(filename);
            if (fl_utf_strcasecmp(ext, ".stl")==0) {
                Iota.addGeometry(filename);
            } else {
                setError("Load Any File", Error::UnknownFileType_STR, filename);
            }
            if (hadError()) {
                showError();
                break;
            }
        }
        if (*fnEnd==0) break;
        fnStart = fnEnd+1;
    }
    gSceneView->redraw();
}


/**
 Experimental stuff.
 */
void IAIota::menuWriteSlice()
{
    char buf[FL_PATH_MAX];

#ifdef _WIN32
    char base[FL_PATH_MAX];
    SHGetSpecialFolderPathA(HWND_DESKTOP, base, CSIDL_DESKTOPDIRECTORY, FALSE);
#else
    const char *base = fl_getenv("HOME");
#endif

    double z = 70.0;
    Iota.gMeshSlice.changeZ(z);
    Iota.gMeshSlice.clear();
    Iota.gMeshSlice.generateFlange(Iota.pMesh);
    Iota.gMeshSlice.tesselateLidFromFlange();
    Iota.gMeshSlice.drawFlat(false, 1, 1, 1);

    snprintf(buf, FL_PATH_MAX, "%s/slice.jpg", base);
    Iota.gMeshSlice.pFramebuffer->saveAsJpeg(buf);

    Iota.gMeshSlice.pFramebuffer->traceOutline(Iota.pCurrentToolpath, zSlider1->value());
    gSceneView->redraw();
}


/**
 Experimental stuff.
 */
void IAIota::menuSliceMesh()
{
    if (!pMachineToolpath)
        pMachineToolpath = new IAMachineToolpath();
    else
        pMachineToolpath->clear();
    double hgt = pMesh->pMax.z() - pMesh->pMin.z();
    // initial height determines stickiness to bed

#if 0
    double maxHgt = hgt;
#else
    double maxHgt = 25;
#endif

    for (double z=0.2; z<maxHgt; z+=0.3) {
        printf("Slicing at z=%g\n", z);
        Iota.gMeshSlice.changeZ(z);
        Iota.gMeshSlice.clear();
        Iota.gMeshSlice.generateFlange(Iota.pMesh);
        Iota.gMeshSlice.tesselateLidFromFlange();
        Iota.gMeshSlice.drawFlat(false, 1, 1, 1);

        uint8_t *rgb = Iota.gMeshSlice.pColorbuffer->getRawImageRGB();

        IAToolpath *tp1 = new IAToolpath(z);
        Iota.gMeshSlice.pFramebuffer->traceOutline(tp1, z);
//        Iota.gMeshSlice.pFramebuffer->saveAsJpeg("/Users/matt/a1.jpg");

        IAToolpath *tp2 = new IAToolpath(z);
        Iota.gMeshSlice.pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        tp1->drawFlat(4);
//        Iota.gMeshSlice.pFramebuffer->saveAsJpeg("/Users/matt/a2.jpg");
        Iota.gMeshSlice.pFramebuffer->unbindFromRendering();
        Iota.gMeshSlice.pFramebuffer->traceOutline(tp2, z);

        IAToolpath *tp3 = new IAToolpath(z);
        Iota.gMeshSlice.pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        tp2->drawFlat(4);
//        Iota.gMeshSlice.pFramebuffer->saveAsJpeg("/Users/matt/a3.jpg");
        Iota.gMeshSlice.pFramebuffer->unbindFromRendering();
        Iota.gMeshSlice.pFramebuffer->traceOutline(tp3, z);

        IAToolpath *tp = pMachineToolpath->createLayer(z);
#if 0
        tp->add(*tp3);
        tp->add(*tp2);
        tp->add(*tp1);
#else
        IAToolpath *b1 = new IAToolpath(z);
        IAToolpath *w1 = new IAToolpath(z);
        IAToolpath *b2 = new IAToolpath(z);
        IAToolpath *w2 = new IAToolpath(z);
        tp2->colorize(rgb, b1, w1);
        tp3->colorize(rgb, b2, w2);
        tp->pList.push_back(new IAToolpathExtruder(0));
        tp->add(*w2);
        tp->add(*w1);
        tp->pList.push_back(new IAToolpathExtruder(1));
        tp->add(*b2);
        tp->add(*b1);
        delete b1;
        delete w1;
        delete b2;
        delete w2;
#endif
        delete tp1;
        delete tp2;
        delete tp3;
        free(rgb);
    }
    pMachineToolpath->saveGCode("/Users/matt/aaa.gcode");
    gSceneView->redraw();
}


/**
 * Just quit the app.
 * \todo At some point, we should probably ask if the user has unsaved changes.
 */
void IAIota::menuQuit()
{
    Iota.gMainWindow->hide();
    Fl::flush();
    exit(0);
}


/**
 * Read a geometry from memory.
 *
 * \param name is needed to define the file type by its extension (test.stl, etc.)
 * \param data binary clone of the file in memory
 * \param size of data in bytes
 */
bool IAIota::addGeometry(const char *name, uint8_t *data, size_t size)
{
    bool ret = false;
    auto reader = IAGeometryReader::findReaderFor(name, data, size);
    if (reader)
        ret = addGeometry(reader);
    return ret;
}


/**
 * Read a geometry from an external file.
 *
 * \param filename the extension of the file name is used to help determine the file type
 */
bool IAIota::addGeometry(const char *filename)
{
    bool ret = false;
    auto reader = IAGeometryReader::findReaderFor(filename);
    if (reader)
        ret = addGeometry(reader);
    return ret;
}


/**
 * Read a geometry from any Reader.
 *
 * \todo Make sure that all buffered data is invalidated when adding another
 *       mesh to the scene.
 *
 * \param read a file reader previously generated by other addGeometry calls
 */
bool IAIota::addGeometry(std::shared_ptr<IAGeometryReader> reader)
{
    bool ret = false;
    delete Iota.pMesh; Iota.pMesh = nullptr;
    auto geometry = reader->load();
    Iota.pMesh = geometry;
    if (pMesh) {
        pMesh->projectTexture(pMesh->pMax.x()*2, pMesh->pMax.y()*2, IA_PROJECTION_FRONT);
        pMesh->projectTexture(3, 1, IA_PROJECTION_CYLINDRICAL);
        pMesh->centerOnPrintbed(&gPrinter);
    }
    return ret;
}


/**
 * Clear all information about previous errors.
 *
 * clearError() and showError() are used like brackets in a high level
 * call from the user interface.
 *
 * A typical user interaction starts with clearError(),
 * followed by a function that may create an error,
 * and finalized by showLastError(), which will present the error to the user,
 * only if one occured.
 *
 * \see IAIota::showLastError(), IAIota::hadError()
 */
void IAIota::clearError()
{
    pError = Error::NoError;
    pErrorBSD = 0;
    pErrorString = nullptr;
}


/**
 * Set an error code that may or may not be shown to the user later.
 *
 * Errors most be meaningful to the user first and foremost!
 *
 * \param loc the error location. This description should be useful to the user,
 *        not the programmer
 * \param err the actual error message. Error messages ending in _BSD will store
 *        the 'errno' at the time of this call.
 * \param str error messages ending in _STR require additonal text to print the
 *        error message, usually a file name
 */
void IAIota::setError(const char *loc, Error err, const char *str)
{
    pErrorLocation = loc;
    pError = err;
    pErrorBSD = errno;
    if (pErrorString)
        ::free((void*)pErrorString);
    pErrorString = str ? strdup(str) : nullptr;
}


/**
 * Return true, if setError() was called since the last call to clearError().
 *
 * \return true if there was an error since the last call to clearError()
 */
bool IAIota::hadError()
{
    return (pError!=Error::NoError);
}


/**
 * Show an alert box with the text of the last error, registered by setError().
 */
void IAIota::showError()
{
    if (hadError()) {
        fl_alert(kErrorMessage[(size_t)pError], pErrorLocation, pErrorString, strerror(pErrorBSD));
    }
}


/**
 * Load a model and a texture that come with the app for an easy example.
 *
 * These are binary assets compiled into the code. There is no need for
 * external files.
 */
void IAIota::loadDemoFiles()
{
    loadTexture("testcard1024.jpg", defaultTexture);
    Iota.addGeometry("default.stl", defaultModel, sizeof(defaultModel));
}


/**
 * Launch our app.
 * \todo remember the window position and size in the preferences
 */
int main (int argc, char **argv)
{
    Fl::scheme("gleam");
	Fl::args(argc, argv);
    Fl::set_color(FL_BACKGROUND_COLOR, 0xeeeeee00);

    Fl::use_high_res_GL(1);

    // TODO: The whole user interface must be in its own class.
    Iota.gMainWindow = createIotaAppWindow();
    Iota.gMainWindow->show();
    Fl::flush();

    Iota.loadDemoFiles();

    gSceneView->redraw();

    return Fl::run();
}

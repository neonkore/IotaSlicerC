//
//  IAPrinterLaser.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_LASERCUTTER_H
#define IA_PRINTER_LASERCUTTER_H

#include "printer/IAPrinter.h"

class Fl_Widget;
class Fl_Choice;
class Fl_Input_Choice;


/**
 * The Lasercutter driver generates slices that can be cut and clued together.
 *
 * This printer driver generates DXF files for laser engravers.
 *
 * No color or multimaterial support.
 */
class IAPrinterLasercutter : public IAPrinter
{
    typedef IAPrinter super;
public:
    IAPrinterLasercutter(const char *name) : super(name) { }

    // ---- direct user interaction
    virtual void userSliceSave() override;
    virtual void userSliceSaveAs() override;
    virtual void userSliceGenerateAll() override;

    // ----
    void sliceAll();
    void saveToolpaths(const char *filename = nullptr);


    virtual void buildSessionSettings() override;

    double layerHeight() { return pLayerHeight; }

protected:
    void userSetLayerHeight(Fl_Input_Choice *w);

private:
    static void userSetLayerHeightCB(Fl_Widget *w, void *d) {
        ((IAPrinterLasercutter*)d)->userSetLayerHeight((Fl_Input_Choice*)w); }
};


#endif /* IA_PRINTER_LASERCUTTER_H */


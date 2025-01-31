// generated by Fast Light User Interface Designer (fluid) version 1.0400

#ifndef IAGUIMain_h
#define IAGUIMain_h
#include <FL/Fl.H>
#include "Iota.h"
#include "printer/IAPrinter.h"
#include "widget/IASceneView.h"
#include "widget/IAGLButton.h"
#include "widget/IAGLRangeSlider.h"
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Terminal.H>
#ifdef __APPLE__
#include <unistd.h>
#include <utime.h>
#endif
#ifdef __LINUX__
#include <unistd.h>
#include <utime.h>
#endif
#ifdef _WIN32
#include <sys/utime.h>
#endif
#include <FL/Fl_Double_Window.H>
extern Fl_Double_Window *wMainWindow;
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Group.H>
extern IASceneView *gSceneView;
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
extern IAGLRangeSlider *zRangeSlider;
#include <FL/Fl_Choice.H>
extern Fl_Choice *wPrinterChoice;
#include <FL/Fl_Tree.H>
extern Fl_Tree *wSessionSettings;
Fl_Double_Window* createIotaAppWindow();
extern Fl_Menu_Item menu_[];
#define wRecentFiles (menu_+4)
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Help_View.H>
Fl_Double_Window* createIotaAboutWindow();
extern Fl_Double_Window *wSettingsWindow;
#include <FL/Fl_Pack.H>
#include <FL/Fl_Browser.H>
extern Fl_Browser *wSettingsPrinterList;
extern Fl_Button *wSettingPrinterAdd;
extern Fl_Tree *wSettingsPrinterProperties;
Fl_Double_Window* createSettingWindow();
#include <FL/Fl_Input.H>
#include <FL/Fl_Input_Choice.H>

class IAVersioneerDialog {
public:
  virtual Fl_Double_Window * createWindow();
  Fl_Double_Window *wVersionWindow;
  Fl_Input *wBasePath;
private:
  inline void cb_wBasePath_i(Fl_Input*, void*);
  static void cb_wBasePath(Fl_Input*, void*);
  inline void cb_fileopen_i(Fl_Button*, void*);
  static void cb_fileopen(Fl_Button*, void*);
public:
  Fl_Input *wMajor;
  Fl_Input *wMinor;
  Fl_Input *wBuild;
  Fl_Input_Choice *wClass;
  static Fl_Menu_Item menu_wClass[];
  Fl_Button *wMajorPlus;
private:
  inline void cb_wMajorPlus_i(Fl_Button*, void*);
  static void cb_wMajorPlus(Fl_Button*, void*);
public:
  Fl_Button *wMinorPlus;
private:
  inline void cb_wMinorPlus_i(Fl_Button*, void*);
  static void cb_wMinorPlus(Fl_Button*, void*);
public:
  Fl_Button *wBuildPlus;
private:
  inline void cb_wBuildPlus_i(Fl_Button*, void*);
  static void cb_wBuildPlus(Fl_Button*, void*);
  inline void cb_Apply_i(Fl_Button*, void*);
  static void cb_Apply(Fl_Button*, void*);
public:
  Fl_Terminal *wTerminal;
  Fl_Input *wFLTKPath;
private:
  inline void cb_wFLTKPath_i(Fl_Input*, void*);
  static void cb_wFLTKPath(Fl_Input*, void*);
  inline void cb_fileopen1_i(Fl_Button*, void*);
  static void cb_fileopen1(Fl_Button*, void*);
  inline void cb_Update_i(Fl_Button*, void*);
  static void cb_Update(Fl_Button*, void*);
public:
  Fl_Input *wArchivePath;
private:
  inline void cb_wArchivePath_i(Fl_Input*, void*);
  static void cb_wArchivePath(Fl_Input*, void*);
  inline void cb_fileopen2_i(Fl_Button*, void*);
  static void cb_fileopen2(Fl_Button*, void*);
  inline void cb_Create_i(Fl_Button*, void*);
  static void cb_Create(Fl_Button*, void*);
  inline void cb_git_i(Fl_Button*, void*);
  static void cb_git(Fl_Button*, void*);
  inline void cb_Close_i(Fl_Button*, void*);
  static void cb_Close(Fl_Button*, void*);
protected:
  virtual void setProjectPath();
  virtual void selectProjectPath();
  virtual void majorPlus();
  virtual void minorPlus();
  virtual void buildPlus();
  virtual void saveSettings();
  virtual void setFLTKPath();
  virtual void selectFLTKPath();
  virtual void updateFLTK();
  virtual void setArchivePath();
  virtual void selectArchivePath();
  virtual void createArchive();
  virtual void gitTag();
  virtual void applySettings();
  IAVersioneerDialog();
  virtual ~IAVersioneerDialog();
};
#endif

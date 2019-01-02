// -*- mode: c++; coding: utf-8 -*-

// Linthesia

// Copyright (c) 2007 Nicholas Piegdon
// Adaptation to GNU/Linux by Oscar Aceña
// See COPYING for license information

#include <string>

#include "OSGraphics.h"
#include "StringUtil.h"
#include "FileSelector.h"
#include "UserSettings.h"
#include "Version.h"
#include "CompatibleSystem.h"
#include "LinthesiaError.h"
#include "Renderer.h"
#include "SharedState.h"
#include "GameState.h"
#include "TitleState.h"

#include <gconfmm.h>

#ifndef GRAPHDIR
#define GRAPHDIR "../graphics"
#endif

using namespace std;

GameStateManager *state_manager;

const static string application_name = "Linthesia";
const static string friendly_app_name = STRING("Linthesia " <<
                                                            LinthesiaVersionString);

const static string error_header1 = "Linthesia detected a";
const static string error_header2 = " problem and must close:\n\n";
const static string error_footer = "\n\nIf you don't think this should have "
                                   "happened, please\ncontact Oscar (on Linthesia sourceforge site) and\n"
                                   "describe what you were doing when the problem\noccurred. Thanks.";

class EdgeTracker {
  public:

    EdgeTracker() :
        active(true),
        just_active(true) {
    }

    void Activate() {
        just_active = true;
        active = true;
    }

    void Deactivate() {
        just_active = false;
        active = false;
    }

    bool IsActive() {
        return active;
    }

    bool JustActivated() {
        bool was_active = just_active;
        just_active = false;
        return was_active;
    }

  private:
    bool active;
    bool just_active;
};

static EdgeTracker window_state;

class DrawingArea : public Gtk::GL::DrawingArea {
  public:

    DrawingArea(const Glib::RefPtr<const Gdk::GL::Config>& config) :
        Gtk::GL::DrawingArea(config) {

        set_events(Gdk::POINTER_MOTION_MASK |
            Gdk::BUTTON_PRESS_MASK |
            Gdk::BUTTON_RELEASE_MASK |
            Gdk::KEY_PRESS_MASK |
            Gdk::KEY_RELEASE_MASK);

        set_can_focus();

        signal_motion_notify_event().connect(sigc::mem_fun(*this, &DrawingArea::on_motion_notify));
        signal_button_press_event().connect(sigc::mem_fun(*this, &DrawingArea::on_button_press));
        signal_button_release_event().connect(sigc::mem_fun(*this, &DrawingArea::on_button_press));
        signal_key_press_event().connect(sigc::mem_fun(*this, &DrawingArea::on_key_press));
        signal_key_release_event().connect(sigc::mem_fun(*this, &DrawingArea::on_key_release));
    }

    bool GameLoop();

  protected:
    virtual void on_realize();
    virtual bool on_configure_event(GdkEventConfigure *event);
    virtual bool on_expose_event(GdkEventExpose *event);

    virtual bool on_motion_notify(GdkEventMotion *event);
    virtual bool on_button_press(GdkEventButton *event);
    virtual bool on_key_press(GdkEventKey *event);
    virtual bool on_key_release(GdkEventKey *event);
};

bool DrawingArea::on_motion_notify(GdkEventMotion *event) {

    state_manager->MouseMove(event->x, event->y);
    return true;
}

bool DrawingArea::on_button_press(GdkEventButton *event) {

    MouseButton b;

    // left and right click allowed
    if (event->button == 1)
        b = MouseLeft;
    else if (event->button == 3)
        b = MouseRight;

        // ignore other buttons
    else
        return false;

    // press or release?
    if (event->type == GDK_BUTTON_PRESS)
        state_manager->MousePress(b);
    else if (event->type == GDK_BUTTON_RELEASE)
        state_manager->MouseRelease(b);
    else
        return false;

    return true;
}

typedef map<int, sigc::connection> ConnectMap;
ConnectMap pressed;

bool __sendNoteOff(int note) {

    ConnectMap::iterator it = pressed.find(note);
    if (it == pressed.end())
        return false;

    sendNote(note, false);
    pressed.erase(it);

    return true;
}

bool DrawingArea::on_key_press(GdkEventKey *event) {
    switch (event->keyval) {
        case GDK_Up: state_manager->KeyPress(KeyUp);
            break;
        case GDK_Down: state_manager->KeyPress(KeyDown);
            break;
        case GDK_Left: state_manager->KeyPress(KeyLeft);
            break;
        case GDK_Right: state_manager->KeyPress(KeyRight);
            break;
        case GDK_space: state_manager->KeyPress(KeySpace);
            break;
        case GDK_Return: state_manager->KeyPress(KeyEnter);
            break;
        case GDK_Escape: state_manager->KeyPress(KeyEscape);
            break;

            // show FPS
        case GDK_F6: state_manager->KeyPress(KeyF6);
            break;

            // increase/decrease octave
        case GDK_greater: state_manager->KeyPress(KeyGreater);
            break;
        case GDK_less: state_manager->KeyPress(KeyLess);
            break;

            // +/- 5 seconds
        case GDK_Page_Down:state_manager->KeyPress(KeyForward);
            break;
        case GDK_Page_Up: state_manager->KeyPress(KeyBackward);
            break;

        case GDK_bracketleft: state_manager->KeyPress(KeyVolumeDown);
            break; // [
        case GDK_bracketright: state_manager->KeyPress(KeyVolumeUp);
            break; // ]

        default:return false;
    }

    return true;
}

bool DrawingArea::on_key_release(GdkEventKey *event) {
    return false;
}

void DrawingArea::on_realize() {
    // we need to call the base on_realize()
    Gtk::GL::DrawingArea::on_realize();

    Glib::RefPtr<Gdk::GL::Window> glwindow = get_gl_window();
    if (!glwindow->gl_begin(get_gl_context()))
        return;

    glwindow->gl_end();
}

bool DrawingArea::on_configure_event(GdkEventConfigure *) {

    Glib::RefPtr<Gdk::GL::Window> glwindow = get_gl_window();
    if (!glwindow->gl_begin(get_gl_context()))
        return false;

    glClearColor(.25, .25, .25, 1.0);
    glClearDepth(1.0);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glShadeModel(GL_SMOOTH);

    glViewport(0, 0, get_width(), get_height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, get_width(), 0, get_height());

    state_manager->Update(window_state.JustActivated());

    glwindow->gl_end();
    return true;
}

bool DrawingArea::on_expose_event(GdkEventExpose *) {

    Glib::RefPtr<Gdk::GL::Window> glwindow = get_gl_window();
    if (!glwindow->gl_begin(get_gl_context()))
        return false;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCallList(1);

    Renderer rend(get_gl_context(), get_pango_context());
    rend.SetVSyncInterval(1);
    state_manager->Draw(rend);

    // swap buffers.
    if (glwindow->is_double_buffered())
        glwindow->swap_buffers();
    else
        glFlush();

    glwindow->gl_end();
    return true;
}

bool DrawingArea::GameLoop() {

    if (window_state.IsActive()) {

        state_manager->Update(window_state.JustActivated());

        Renderer rend(get_gl_context(), get_pango_context());
        rend.SetVSyncInterval(1);

        state_manager->Draw(rend);
    }

    return true;
}

int main(int argc, char *argv[]) {
    Gtk::Main main_loop(argc, argv);
    Gtk::GL::init(argc, argv);

    state_manager = new GameStateManager(
        Compatible::GetDisplayWidth(),
        Compatible::GetDisplayHeight()
    );

    try {
        std::string midi_file;

        UserSetting::Initialize("Linthesia");

        if (argc > 1)
            midi_file = argv[1];

        // strip any leading or trailing quotes from the filename
        // argument (to match the format returned by the open-file
        // dialog later).
        if (!midi_file.empty() &&
            midi_file[0] == '\"')
            midi_file = midi_file.substr(1, midi_file.length() - 1);

        if (!midi_file.empty() &&
            midi_file[midi_file.length() - 1] == '\"')
            midi_file = midi_file.substr(0, midi_file.length() - 1);

        Midi *midi{nullptr};

        // attempt to open the midi file given on the command line first
        if (!midi_file.empty()) {
            try {
                midi = new Midi(Midi::ReadFromFile(midi_file));
            }
            catch (const MidiError& e) {
                string wrapped_description = STRING("Problem while loading file: " <<
                                                                                   midi_file <<
                                                                                   "\n") + e.GetErrorDescription();
                Compatible::ShowError(wrapped_description);
                midi_file.clear();
                delete midi;
            }
        }

        // if midi couldn't be opened from command line filename or there
        // simply was no command line filename, use a "file open" dialog.
        std::string midi_name;
        if (midi_file.empty()) {
            while (!midi) {
                auto [req_path, req_name] = FileSelector::RequestMidiFilename();

                if (!req_path.empty()) {
                    midi_file = req_path;
                    midi_name = req_name;
                    try {
                        midi = new Midi(Midi::ReadFromFile(req_path));
                    } catch (const MidiError& e) {
                        string wrapped_description = \
          STRING("Problem while loading file: " <<
                                                req_name <<
                                                "\n") + e.GetErrorDescription();
                        Compatible::ShowError(wrapped_description);
                        delete midi;
                    }
                } else {
                    // they pressed cancel, so they must not want to run
                    // the app anymore.
                    return 0;
                }
            }
        }

        Glib::RefPtr<Gdk::GL::Config> glconfig;

        // try double-buffered visual
        glconfig = Gdk::GL::Config::create(Gdk::GL::MODE_RGB |
            Gdk::GL::MODE_DEPTH |
            Gdk::GL::MODE_DOUBLE);
        if (!glconfig) {
            cerr << "*** Cannot find the double-buffered visual.\n"
                 << "*** Trying single-buffered visual.\n";

            // try single-buffered visual
            glconfig = Gdk::GL::Config::create(Gdk::GL::MODE_RGB |
                Gdk::GL::MODE_DEPTH);
            if (!glconfig) {
                string description = STRING(error_header1 <<
                                                          " OpenGL" <<
                                                          error_header2 <<
                                                          "Cannot find any OpenGL-capable visual." <<
                                                          error_footer);
                Compatible::ShowError(description);
                return 1;
            }
        }

        Gtk::Window window;
        DrawingArea da(glconfig);
        window.add(da);
        window.show_all();
        window.move(Compatible::GetDisplayLeft() + Compatible::GetDisplayWidth() / 2,
                    Compatible::GetDisplayTop() + Compatible::GetDisplayHeight() / 2);


        // Init DHMS thread once for the whole program
        DpmsThread *dpms_thread = new DpmsThread();

        // do this after gl context is created (ie. after da realized)
        SharedState state;
        state.song_title = FileSelector::TrimFilename(midi_file);
        state.midi = midi;
        state.dpms_thread = dpms_thread;
        state_manager->SetInitialState(new TitleState(state));

        window.fullscreen();
        window.set_title(STRING("Linthesia " << LinthesiaVersionString));

        window.set_icon_from_file(string(GRAPHDIR) + "/app_icon.ico");

        // get refresh rate from user settings
        string key = "refresh_rate";
        int rate = 65;
        string user_rate = UserSetting::Get(key, "");
        if (user_rate.empty()) {
            user_rate = STRING(rate);
            UserSetting::Set(key, user_rate);
        } else {
            istringstream iss(user_rate);
            if (not(iss >> rate)) {
                Compatible::ShowError("Invalid setting for '" + key + "' key.\n\nReset to default value when reload.");
                UserSetting::Set(key, "");
            }
        }

        Glib::signal_timeout().connect(sigc::mem_fun(da, &DrawingArea::GameLoop), 1000 / rate);

        main_loop.run(window);
        window_state.Deactivate();

        delete dpms_thread;
        return 0;
    }

    catch (const LinthesiaError& e) {
        string wrapped_description = STRING(error_header1 <<
                                                          error_header2 <<
                                                          e.GetErrorDescription() <<
                                                          error_footer);
        Compatible::ShowError(wrapped_description);
    }

    catch (const MidiError& e) {
        string wrapped_description = STRING(error_header1 <<
                                                          " MIDI" <<
                                                          error_header2 <<
                                                          e.GetErrorDescription() <<
                                                          error_footer);
        Compatible::ShowError(wrapped_description);
    }

    catch (const Gnome::Conf::Error& e) {
        string wrapped_description = STRING(error_header1 <<
                                                          " Gnome::Conf::Error" <<
                                                          error_header2 <<
                                                          e.what() <<
                                                          error_footer);
        Compatible::ShowError(wrapped_description);
    }

    catch (const exception& e) {
        string wrapped_description = STRING("Linthesia detected an unknown "
                                            "problem and must close!  '" <<
                                                                         e.what() << "'" << error_footer);
        Compatible::ShowError(wrapped_description);
    }

    catch (...) {
        string wrapped_description = STRING("Linthesia detected an unknown "
                                            "problem and must close!" <<
                                                                      error_footer);
        Compatible::ShowError(wrapped_description);
    }

    return 1;
}


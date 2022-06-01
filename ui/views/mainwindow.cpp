#include "mainwindow.h"
#include "../controls/progressdialog.h"
#include "../controls/progresstracker.h"
#include "preferencesdialog.h"
#include "shortcutsdialog.h"

using namespace NickvisionApplication::Models;
using namespace NickvisionApplication::UI;
using namespace NickvisionApplication::UI::Controls;
using namespace NickvisionApplication::UI::Views;
using namespace NickvisionApplication::Update;

MainWindow::MainWindow(Configuration& configuration) : Widget{"/ui/views/mainwindow.xml", "adw_winMain"}, m_configuration{configuration}, m_updater{"https://raw.githubusercontent.com/nlogozzo/NickvisionApplication/main/UpdateConfig.json", { "2022.5.0" }}, m_opened{false}
{
    //==Signals==//
    g_signal_connect(m_gobj, "show", G_CALLBACK((void (*)(GtkWidget*, gpointer*))[](GtkWidget* widget, gpointer* data) { reinterpret_cast<MainWindow*>(data)->onStartup(); }), this);
    //==App Actions==//
    //Open Folder
    m_gio_actOpenFolder = g_simple_action_new("openFolder", nullptr);
    g_signal_connect(m_gio_actOpenFolder, "activate", G_CALLBACK((void (*)(GSimpleAction*, GVariant*, gpointer*))[](GSimpleAction* action, GVariant* parameter, gpointer* data) { reinterpret_cast<MainWindow*>(data)->openFolder(); }), this);
    g_action_map_add_action(G_ACTION_MAP(m_gobj), G_ACTION(m_gio_actOpenFolder));
    //Close Folder
    m_gio_actCloseFolder = g_simple_action_new("closeFolder", nullptr);
    g_signal_connect(m_gio_actCloseFolder, "activate", G_CALLBACK((void (*)(GSimpleAction*, GVariant*, gpointer*))[](GSimpleAction* action, GVariant* parameter, gpointer* data) { reinterpret_cast<MainWindow*>(data)->closeFolder(); }), this);
    g_action_map_add_action(G_ACTION_MAP(m_gobj), G_ACTION(m_gio_actCloseFolder));
    //==Help Actions==//
    //Check for Updates
    m_gio_actUpdate = g_simple_action_new("update", nullptr);
    g_signal_connect(m_gio_actUpdate, "activate", G_CALLBACK((void (*)(GSimpleAction*, GVariant*, gpointer*))[](GSimpleAction* action, GVariant* parameter, gpointer* data) { reinterpret_cast<MainWindow*>(data)->update(); }), this);
    g_action_map_add_action(G_ACTION_MAP(m_gobj), G_ACTION(m_gio_actUpdate));
    //Preferences
    m_gio_actPreferences = g_simple_action_new("preferences", nullptr);
    g_signal_connect(m_gio_actPreferences, "activate", G_CALLBACK((void (*)(GSimpleAction*, GVariant*, gpointer*))[](GSimpleAction* action, GVariant* parameter, gpointer* data) { reinterpret_cast<MainWindow*>(data)->preferences(); }), this);
    g_action_map_add_action(G_ACTION_MAP(m_gobj), G_ACTION(m_gio_actPreferences));
    //Keyboard Shortcuts
    m_gio_actKeyboardShortcuts = g_simple_action_new("keyboardShortcuts", nullptr);
    g_signal_connect(m_gio_actKeyboardShortcuts, "activate", G_CALLBACK((void (*)(GSimpleAction*, GVariant*, gpointer*))[](GSimpleAction* action, GVariant* parameter, gpointer* data) { reinterpret_cast<MainWindow*>(data)->keyboardShortcuts(); }), this);
    g_action_map_add_action(G_ACTION_MAP(m_gobj), G_ACTION(m_gio_actKeyboardShortcuts));
    //Changelog
    m_gio_actChangelog = g_simple_action_new("changelog", nullptr);
    g_signal_connect(m_gio_actChangelog, "activate", G_CALLBACK((void (*)(GSimpleAction*, GVariant*, gpointer*))[](GSimpleAction* action, GVariant* parameter, gpointer* data) { reinterpret_cast<MainWindow*>(data)->changelog(); }), this);
    g_action_map_add_action(G_ACTION_MAP(m_gobj), G_ACTION(m_gio_actChangelog));
    //About
    m_gio_actAbout = g_simple_action_new("about", nullptr);
    g_signal_connect(m_gio_actAbout, "activate", G_CALLBACK((void (*)(GSimpleAction*, GVariant*, gpointer*))[](GSimpleAction* action, GVariant* parameter, gpointer* data) { reinterpret_cast<MainWindow*>(data)->about(); }), this);
    g_action_map_add_action(G_ACTION_MAP(m_gobj), G_ACTION(m_gio_actAbout));
    //==Help Menu Button==//
    GtkBuilder* builderMenu{gtk_builder_new_from_resource("/ui/views/menuhelp.xml")};
    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(gtk_builder_get_object(m_builder, "gtk_btnMenuHelp")), G_MENU_MODEL(gtk_builder_get_object(builderMenu, "gio_menuHelp")));
    g_object_unref(builderMenu);
}

MainWindow::~MainWindow()
{
    m_configuration.save();
    gtk_window_destroy(GTK_WINDOW(m_gobj));
}

void MainWindow::showMaximized()
{
    gtk_widget_show(m_gobj);
    gtk_window_maximize(GTK_WINDOW(m_gobj));
}

void MainWindow::onStartup()
{
    if(!m_opened)
    {
        //==Set Action Shortcuts==//
        //Open Folder
        gtk_application_set_accels_for_action(gtk_window_get_application(GTK_WINDOW(m_gobj)), "win.openFolder", new const char*[2]{ "<Ctrl>o", nullptr });
        //Close Folder
        gtk_application_set_accels_for_action(gtk_window_get_application(GTK_WINDOW(m_gobj)), "win.closeFolder", new const char*[2]{ "<Ctrl>w", nullptr });
        //About
        gtk_application_set_accels_for_action(gtk_window_get_application(GTK_WINDOW(m_gobj)), "win.about", new const char*[2]{ "F1", nullptr });
        //==Load Configuration==//
        m_configuration.setIsFirstTimeOpen(false);
        m_configuration.save();
        //==Check for Updates==//
        ProgressTracker* progTrackerUpdate{new ProgressTracker("Checking for updates...", [&]() { m_updater.checkForUpdates(); }, [&]()
        {
            if(m_updater.getUpdateAvailable())
            {
                sendToast("A new update is avaliable.");
            }
        })};
        adw_header_bar_pack_end(ADW_HEADER_BAR(gtk_builder_get_object(m_builder, "adw_headerBar")), progTrackerUpdate->gobj());
        progTrackerUpdate->show();
        m_opened = true;
    }
}

void MainWindow::openFolder()
{
    GtkFileChooserNative* openFolderDialog{gtk_file_chooser_native_new("Open Folder", GTK_WINDOW(m_gobj), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "_Open", "_Cancel")};
    gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(openFolderDialog), true);
    g_signal_connect(openFolderDialog, "response", G_CALLBACK((void (*)(GtkNativeDialog*, gint, gpointer*))([](GtkNativeDialog* dialog, gint response_id, gpointer* data)
    {
        if(response_id == GTK_RESPONSE_ACCEPT)
        {
            MainWindow* mainWindow{reinterpret_cast<MainWindow*>(data)};
            GtkFileChooser* chooser{GTK_FILE_CHOOSER(dialog)};
            GFile* file{gtk_file_chooser_get_file(chooser)};
            std::string path{g_file_get_path(file)};
            g_object_unref(file);
            adw_window_title_set_subtitle(ADW_WINDOW_TITLE(gtk_builder_get_object(GTK_BUILDER(mainWindow->m_builder), "adw_title")), path.c_str());
            gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(mainWindow->m_builder, "gtk_btnCloseFolder")), true);
        }
        g_object_unref(dialog);
    })), this);
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(openFolderDialog));
}

void MainWindow::closeFolder()
{
    adw_window_title_set_subtitle(ADW_WINDOW_TITLE(gtk_builder_get_object(GTK_BUILDER(m_builder), "adw_title")), nullptr);
    gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(m_builder, "gtk_btnCloseFolder")), false);
}

void MainWindow::update()
{
    if(m_updater.getUpdateAvailable())
    {
        GtkWidget* updateDialog{gtk_message_dialog_new(GTK_WINDOW(m_gobj), GtkDialogFlags(GTK_DIALOG_MODAL),
            GTK_MESSAGE_INFO, GTK_BUTTONS_YES_NO, "Update Available")};
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(updateDialog), std::string("\n===V" + m_updater.getLatestVersion().toString() + " Changelog===\n" + m_updater.getChangelog() + "\n\nApplication can automatically download the update tar.gz file to your Downloads directory. Would you like to continue?").c_str());
        g_signal_connect(updateDialog, "response", G_CALLBACK((void (*)(GtkDialog*, gint, gpointer*))([](GtkDialog* dialog, gint response_id, gpointer* data)
        {
            gtk_window_destroy(GTK_WINDOW(dialog));
            if(response_id == GTK_RESPONSE_YES)
            {
                MainWindow* mainWindow{reinterpret_cast<MainWindow*>(data)};
                ProgressTracker* progTrackerDownloading{new ProgressTracker("Downloading the update...", [mainWindow]() { mainWindow->m_updater.update(); }, [mainWindow]()
                {
                    if(mainWindow->m_updater.getUpdateSuccessful())
                    {
                        mainWindow->sendToast("Update downloaded successfully. Please visit your Downloads folder to unpack and run the new update.");
                    }
                    else
                    {
                        mainWindow->sendToast("Error: Unable to download the update.");
                    }
                })};
                adw_header_bar_pack_end(ADW_HEADER_BAR(gtk_builder_get_object(mainWindow->m_builder, "adw_headerBar")), progTrackerDownloading->gobj());
                progTrackerDownloading->show();
            }
        })), this);
        gtk_widget_show(updateDialog);
    }
    else
    {
        sendToast("There is no update at this time. Please try again later.");
    }
}

void MainWindow::preferences()
{
    PreferencesDialog* preferencesDialog{new PreferencesDialog(m_gobj, m_configuration)};
    std::pair<PreferencesDialog*, MainWindow*>* pointers{new std::pair<PreferencesDialog*, MainWindow*>(preferencesDialog, this)};
    g_signal_connect(preferencesDialog->gobj(), "hide", G_CALLBACK((void (*)(GtkWidget*, gpointer*))([](GtkWidget* widget, gpointer* data)
    {
        std::pair<PreferencesDialog*, MainWindow*>* pointers{reinterpret_cast<std::pair<PreferencesDialog*, MainWindow*>*>(data)};
        delete pointers->first;
        if(pointers->second->m_configuration.getTheme() == Theme::System)
        {
           adw_style_manager_set_color_scheme(adw_style_manager_get_default(), ADW_COLOR_SCHEME_PREFER_LIGHT);
        }
        else if(pointers->second->m_configuration.getTheme() == Theme::Light)
        {
           adw_style_manager_set_color_scheme(adw_style_manager_get_default(), ADW_COLOR_SCHEME_FORCE_LIGHT);
        }
        else if(pointers->second->m_configuration.getTheme() == Theme::Dark)
        {
           adw_style_manager_set_color_scheme(adw_style_manager_get_default(), ADW_COLOR_SCHEME_FORCE_DARK);
        }
        delete pointers;
    })), pointers);
    preferencesDialog->show();
}

void MainWindow::keyboardShortcuts()
{
    ShortcutsDialog* shortcutsDialog{new ShortcutsDialog(m_gobj)};
    g_signal_connect(shortcutsDialog->gobj(), "hide", G_CALLBACK((void (*)(GtkWidget*, gpointer*))([](GtkWidget* widget, gpointer* data)
    {
        ShortcutsDialog* dialog{reinterpret_cast<ShortcutsDialog*>(data)};
        delete dialog;
    })), shortcutsDialog);
    shortcutsDialog->show();
}

void MainWindow::changelog()
{
    GtkWidget* changelogDialog{gtk_message_dialog_new(GTK_WINDOW(m_gobj), GtkDialogFlags(GTK_DIALOG_MODAL),
        GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "What's New?")};
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(changelogDialog), "- Initial Release");
    g_signal_connect(changelogDialog, "response", G_CALLBACK(gtk_window_destroy), nullptr);
    gtk_widget_show(changelogDialog);
}

void MainWindow::about()
{
    const char* authors[]{ "Nicholas Logozzo", nullptr };
    gtk_show_about_dialog(GTK_WINDOW(m_gobj), "program-name", "NickvisionApplication", "version", "2022.5.0", "comments", "A template for creating Nickvision applications.",
                          "copyright", "(C) Nickvision 2021-2022", "license-type", GTK_LICENSE_GPL_3_0, "website", "https://github.com/nlogozzo/NickvisionApplication", "website-label", "GitHub",
                          "authors", authors, nullptr);
}

void MainWindow::sendToast(const std::string& message)
{
    AdwToast* toast{adw_toast_new(message.c_str())};
    adw_toast_overlay_add_toast(ADW_TOAST_OVERLAY(gtk_builder_get_object(m_builder, "adw_toastOverlay")), toast);
}

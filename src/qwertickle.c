/*
 * Qwertickle - typewriter sounds for keyboard input.
 * Original project by Wei-Ning Huang (AZ), GPL-2.0-or-later.
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <glib.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <libayatana-appindicator/app-indicator.h>

#include "qwertickle.h"

#ifndef DATADIR
#define DATADIR "/usr/share/qwertickle"
#endif

enum {
    SOUND_KEY = 0,
    SOUND_RETURN,
    SOUND_SPACE,
    SOUND_BACKSPACE
};

static const gchar *sound_files[] = {
    DATADIR "/typewriter-key.mp3",
    DATADIR "/typewriter-return.mp3",
    DATADIR "/typewriter-space-bar.mp3",
    DATADIR "/typewriter-backspace.mp3"
};

static AppIndicator *indicator;
static GtkWidget *enable_item;
static GThread *input_thread;
static gchar *device_path;
static gint stop_requested = 0;

static gboolean play_sound(gpointer data);
static void start_listener(void);
static void stop_listener(void);

static void show_error(const gchar *title, const gchar *message)
{
    GtkWidget *dialog = gtk_message_dialog_new(
        NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
        "%s", title);
    gtk_message_dialog_format_secondary_text(
        GTK_MESSAGE_DIALOG(dialog), "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static gboolean on_gst_message(GstBus *bus, GstMessage *message, gpointer data)
{
    GstElement *player = GST_ELEMENT(data);
    (void)bus;

    switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_EOS:
    case GST_MESSAGE_ERROR:
        gst_element_set_state(player, GST_STATE_NULL);
        gst_object_unref(player);
        return G_SOURCE_REMOVE;
    default:
        return G_SOURCE_CONTINUE;
    }
}

static gboolean play_sound(gpointer data)
{
    const guint sound_index = GPOINTER_TO_UINT(data);
    gchar *uri;
    GError *error = NULL;
    GstElement *player;
    GstBus *bus;

    if (sound_index >= G_N_ELEMENTS(sound_files))
        return G_SOURCE_REMOVE;

    uri = gst_filename_to_uri(sound_files[sound_index], &error);
    if (uri == NULL) {
        g_warning("Cannot create sound URI: %s",
                  error ? error->message : "unknown error");
        g_clear_error(&error);
        return G_SOURCE_REMOVE;
    }

    player = gst_element_factory_make("playbin", NULL);
    if (player == NULL) {
        g_warning("GStreamer playbin plugin is unavailable");
        g_free(uri);
        return G_SOURCE_REMOVE;
    }

    g_object_set(player, "uri", uri, NULL);
    g_free(uri);

    bus = gst_element_get_bus(player);
    gst_bus_add_watch(bus, on_gst_message, player);
    gst_object_unref(bus);

    if (gst_element_set_state(player, GST_STATE_PLAYING) ==
        GST_STATE_CHANGE_FAILURE) {
        gst_element_set_state(player, GST_STATE_NULL);
        gst_object_unref(player);
    }

    return G_SOURCE_REMOVE;
}

static guint sound_for_key(unsigned int code)
{
    switch (code) {
    case KEY_ENTER:
    case KEY_KPENTER:
        return SOUND_RETURN;
    case KEY_SPACE:
        return SOUND_SPACE;
    case KEY_BACKSPACE:
        return SOUND_BACKSPACE;
    default:
        return SOUND_KEY;
    }
}

static gpointer input_worker(gpointer unused)
{
    struct input_event event;
    struct pollfd descriptor;
    gchar name[256] = "unknown keyboard";
    int fd;
    ssize_t bytes;
    (void)unused;

    fd = open(device_path, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
    if (fd < 0) {
        gchar *message = g_strdup_printf(
            "Could not open %s: %s\n\nCheck that your user has permission "
            "to read this keyboard device.", device_path, g_strerror(errno));
        g_main_context_invoke(NULL, (GSourceFunc)show_error, message);
        return NULL;
    }

    if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0)
        g_strlcpy(name, device_path, sizeof(name));
    g_message("Listening to keyboard: %s", name);

    descriptor.fd = fd;
    descriptor.events = POLLIN;
    descriptor.revents = 0;

    while (!g_atomic_int_get(&stop_requested)) {
        int ready = poll(&descriptor, 1, 250);

        if (ready < 0) {
            if (errno == EINTR)
                continue;
            break;
        }
        if (ready == 0)
            continue;
        if (descriptor.revents & (POLLERR | POLLHUP | POLLNVAL))
            break;

        bytes = read(fd, &event, sizeof(event));
        if (bytes == (ssize_t)sizeof(event) &&
            event.type == EV_KEY && event.value == 1) {
            guint sound_index = sound_for_key(event.code);
            g_main_context_invoke(NULL, play_sound,
                                  GUINT_TO_POINTER(sound_index));
        }
    }

    close(fd);
    return NULL;
}

static void start_listener(void)
{
    if (device_path == NULL || input_thread != NULL)
        return;

    g_atomic_int_set(&stop_requested, 0);
    input_thread = g_thread_new("qwertickle-input", input_worker, NULL);
}

static void stop_listener(void)
{
    if (input_thread == NULL)
        return;

    g_atomic_int_set(&stop_requested, 1);
    g_thread_join(input_thread);
    input_thread = NULL;
}

static void on_enable_toggled(GtkCheckMenuItem *item, gpointer data)
{
    (void)data;
    if (gtk_check_menu_item_get_active(item))
        start_listener();
    else
        stop_listener();
}

static void on_choose_device(GtkMenuItem *item, gpointer data)
{
    GtkWidget *dialog;
    (void)item;
    (void)data;

    dialog = gtk_file_chooser_dialog_new(
        "Select keyboard input device", NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Select", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(
        GTK_FILE_CHOOSER(dialog), "/dev/input");

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *selected = gtk_file_chooser_get_filename(
            GTK_FILE_CHOOSER(dialog));
        stop_listener();
        g_free(device_path);
        device_path = selected;
        if (gtk_check_menu_item_get_active(
                GTK_CHECK_MENU_ITEM(enable_item)))
            start_listener();
    }

    gtk_widget_destroy(dialog);
}

static void on_about(GtkMenuItem *item, gpointer data)
{
    const gchar *authors[] = {
        "Wei-Ning Huang (AZ)",
        "Modernized GTK/GStreamer/Wayland port",
        NULL
    };
    GtkWidget *dialog;
    (void)item;
    (void)data;

    dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Qwertickle");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), PACKAGE_VERSION);
    gtk_about_dialog_set_comments(
        GTK_ABOUT_DIALOG(dialog),
        "A typewriter sound scheme for your keyboard.");
    gtk_about_dialog_set_license_type(
        GTK_ABOUT_DIALOG(dialog), GTK_LICENSE_GPL_2_0);
    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), authors);
    gtk_about_dialog_set_website(
        GTK_ABOUT_DIALOG(dialog), "https://github.com/Aitjcize/Qwertickle");
    gtk_window_set_icon_from_file(
        GTK_WINDOW(dialog), DATADIR "/qwertickle.png", NULL);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void on_quit(GtkMenuItem *item, gpointer data)
{
    (void)item;
    (void)data;
    stop_listener();
    gtk_main_quit();
}

static gboolean prompt_for_device(gpointer data)
{
    on_choose_device(NULL, NULL);
    return G_SOURCE_REMOVE;
}

int main(int argc, char **argv)
{
    GtkWidget *menu;
    GtkWidget *choose_item;
    GtkWidget *about_item;
    GtkWidget *separator;
    GtkWidget *quit_item;

    gtk_init(&argc, &argv);
    gst_init(&argc, &argv);

    indicator = app_indicator_new(
        "qwertickle", "input-keyboard",
        APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    app_indicator_set_icon_full(
        indicator, DATADIR "/qwertickle.png", "Qwertickle");

    menu = gtk_menu_new();
    enable_item = gtk_check_menu_item_new_with_label("Enabled");
    choose_item = gtk_menu_item_new_with_label("Select keyboard…");
    about_item = gtk_menu_item_new_with_label("About Qwertickle");
    separator = gtk_separator_menu_item_new();
    quit_item = gtk_menu_item_new_with_label("Quit");

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), enable_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), choose_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), about_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit_item);

    g_signal_connect(enable_item, "toggled",
                     G_CALLBACK(on_enable_toggled), NULL);
    g_signal_connect(choose_item, "activate",
                     G_CALLBACK(on_choose_device), NULL);
    g_signal_connect(about_item, "activate", G_CALLBACK(on_about), NULL);
    g_signal_connect(quit_item, "activate", G_CALLBACK(on_quit), NULL);

    gtk_check_menu_item_set_active(
        GTK_CHECK_MENU_ITEM(enable_item), TRUE);
    gtk_widget_show_all(menu);
    app_indicator_set_menu(indicator, GTK_MENU(menu));

    g_idle_add(prompt_for_device, NULL);
    gtk_main();

    stop_listener();
    g_clear_pointer(&device_path, g_free);
    g_object_unref(indicator);
    return 0;
}

/*
 * Mail Notification Toy
 * By Zane Ashby -- http://github.com/ZaneA/MailNotify
 *
 * This is a very simple GTK application that reads numbers
 * from stdin separated by newlines and displays an image
 * based on that number (ie. 0.gif, 1.gif etc)
 *
 * Intended to be used with the number gif images from
 * http://konachan.com - Download each from 0-9 and place
 * them in the same folder as this application
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/select.h>

GtkWidget *window;              /* Holds the GTK Window */
GtkWidget *image;               /* Holds the current image */
int current = -1;               /* Holds the current number */

/* Loads a new image from a number */
void load_image_from_number(int num) {
    if (image) {
        /* If an image is loaded, then unload it first */
        gtk_widget_destroy(image);
        image = NULL;
    }

    /* Build the filename buffer */
    char filename[32];
    sprintf(filename, "%i.gif", num);

    /* Load up the image and add it to the window */
    image = gtk_image_new_from_file(filename);
    gtk_container_add(GTK_CONTAINER(window), image);
    gtk_widget_show(image);
}

// From StackOverflow - Thanks to digitalarbeiter
// http://stackoverflow.com/questions/1594251/how-to-check-if-stdin-is-still-opened-without-blocking
// Checks if a file descriptor is ready to be read
int is_ready(int fd) {
    fd_set fdset;
    struct timeval timeout;
    int ret;
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;
    return select(1, &fdset, NULL, NULL, &timeout) == 1 ? 1 : 0;
}

/* This function is called every 100 milliseconds */
static gboolean time_handler(GtkWidget *widget) {
    if (widget->window == NULL) return FALSE;

    if (is_ready(fileno(stdin))) { /* stdin is ready */
        int num;
        fscanf(stdin, "%i", &num);

        if (num != current) {
            /* If number read is different to the current one */
            load_image_from_number(num); /* Load a new image */
            current = num;
        }

        /* Update the window title, not visible anyway */
        char temp[64];
        sprintf(temp, "mail notify - %i %s", num, ((num == 1) ? "email" : "emails"));
        gtk_window_set_title(GTK_WINDOW(window), temp);
    }

    return TRUE;
}

/* From the alphademo in GTK sources */
gboolean supports_alpha = FALSE;
static void screen_changed(GtkWidget *widget, GdkScreen *old_screen, gpointer userdata) {
    GdkScreen *screen = gtk_widget_get_screen(widget);
    GdkColormap *colormap = gdk_screen_get_rgba_colormap(screen);

    if (!colormap) {
        colormap = gdk_screen_get_rgb_colormap(screen);
        supports_alpha = FALSE;
    } else {
        supports_alpha = TRUE;
    }

    gtk_widget_set_colormap(widget, colormap);
}

/* Overrides the drawing when window is "exposed" by X11 */
static gboolean expose(GtkWidget *widget, GdkEventExpose *event, gpointer userdata) {
    cairo_t *cr = gdk_cairo_create(widget->window);

    if (supports_alpha)
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
    else
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);

    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    cairo_destroy(cr);

    return FALSE;
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /* Set up a few switches for the window */
    gtk_window_set_title(GTK_WINDOW(window), "email notify");
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_widget_set_app_paintable(window, TRUE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_keep_below(GTK_WINDOW(window), TRUE);
    gtk_window_stick(GTK_WINDOW(window));

    /* Connect up the signals */
    g_signal_connect(GTK_OBJECT(window), "expose-event", G_CALLBACK(expose), NULL);
    g_signal_connect(GTK_OBJECT(window), "screen-changed", G_CALLBACK(screen_changed), NULL);

    gtk_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

    gtk_container_border_width(GTK_CONTAINER(window), 10);

    screen_changed(window, NULL, NULL);

    g_timeout_add(100, (GSourceFunc)time_handler, (gpointer)window);
    time_handler(window);

    gtk_widget_show(window);

    /* Go */
    gtk_main();

    return 0;
}

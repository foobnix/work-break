#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include "fullscreen.h"
#include "core.h"

GtkWidget * time_label;
GtkWidget *f_window;

void * event(GtkWidget *widget, GdkEvent *event){
    //c_on_any_event();
    printf("event type %i \n", event->type);
}

//static gboolean key_press(GtkWidget *win, GdkEventKey *ev, gpointer data) {
//    if (ev->keyval == GDK_Escape) {
//        //gtk_widget_destroy(win);
//        GtkWindow *window = (GtkWindow *) data;
//        //gtk_window_unfullscreen(window);
//        gtk_widget_hide_all(window);
//    }
//    return FALSE;
//}

static GdkPixbuf * get_screenshot() {
    GdkPixbuf *screenshot;
    GdkWindow *root_window;
    gint x_orig, y_orig;
    gint width, height;
    root_window = gdk_get_default_root_window();
    gdk_drawable_get_size(root_window, &width, &height);
    gdk_window_get_origin(root_window, &x_orig, &y_orig);

    screenshot = gdk_pixbuf_get_from_drawable(NULL, root_window, NULL, x_orig, y_orig, 0, 0, width, height);
    return screenshot;
}

static void colorinvert_picture(GdkPixbuf *pb) {
    int ht, wt;
    int i = 0, j = 0;
    int rowstride = 0;
    int bpp = 0;
    guchar *pixel;

    if (gdk_pixbuf_get_bits_per_sample(pb) != 8) //we handle only 24 bit images.
        return;                               //look at 3 bytes per pixel.

    bpp = 3;                  //getting attributes of height,
    ht = gdk_pixbuf_get_height(pb);   //width, and bpp.Also get pointer
    wt = gdk_pixbuf_get_width(pb);    //to pixels.
    pixel = gdk_pixbuf_get_pixels(pb);
    rowstride = wt * bpp;
    double avg = 0;
    for (i = 0; i < ht; i++)        //iterate over the height of image.
        for (j = 0; j < rowstride; j += bpp)  //read every pixel in the row.skip
                //bpp bytes
                {

             if (i % 10 == 0) {
//                pixel[i * rowstride + j + 0] = 255 - pixel[i * rowstride + j + 0];
//                pixel[i * rowstride + j + 1] = 255 - pixel[i * rowstride + j + 1];
//                pixel[i * rowstride + j + 2] = 255 - pixel[i * rowstride + j + 2];

            avg += pixel[i * rowstride + j + 0] + pixel[i * rowstride + j + 1] + pixel[i * rowstride + j + 2];
            avg /= 3.00;
            avg = ((int) (avg)) % 256;

            pixel[i * rowstride + j + 0] = (int) avg;
            pixel[i * rowstride + j + 1] = (int) avg;
            pixel[i * rowstride + j + 2] = (int) avg;
             }
        }
    return;
}


void f_hide() {
    //gtk_window_unfullscreen(GTK_WINDOW(f_window) );
    gtk_widget_hide(f_window);
}
void f_show_all() {
    //gtk_window_fullscreen(GTK_WINDOW(f_window) );
    gtk_widget_show_all(f_window);
}

void any_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    printf("Event \n");
    printf("%i", event->type);
}

void fullscreen_show_init() {
    f_window = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_window_set_position(GTK_WINDOW(f_window), GTK_WIN_POS_CENTER);

    //gtk_window_set_title(GTK_WINDOW (window), "Hello World");
    //gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_keep_above(GTK_WINDOW (f_window), TRUE);

    GdkScreen *screen = gdk_screen_get_default();
    GdkRectangle monitor;
    gint root_monitor = 0;

    gdk_screen_get_monitor_geometry(screen, root_monitor, &monitor);

    // GdkWindow *root =  gdk_get_default_root_window();
    // int w = gdk_window_get_width(root);
    // int h = gdk_window_get_height(root);

    int w = gdk_screen_get_width(screen);
    int h = gdk_screen_get_height(screen);
    //int w = 100;
    //int h = 100;

    gtk_window_set_default_size(GTK_WINDOW (f_window), w, h);
    //gtk_window_fullscreen(GTK_WINDOW(f_window) );

    GtkWidget *root_window = gdk_get_default_root_window();
    //g_signal_connect(GTK_STATUS_ICON (root_window), "f_window", GTK_SIGNAL_FUNC (any_event), NULL);

    GtkWidget *line1 = gtk_label_new("");
    GtkWidget *line2 = gtk_label_new("");

    GtkWidget *exit = gtk_button_new();
    gtk_button_set_label(GTK_BUTTON(exit), "Exit");

    g_signal_connect(exit, "clicked", G_CALLBACK(f_hide), NULL);

    GtkWidget * box = gtk_vbox_new(FALSE, 0);

    GtkWidget *text = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(text),
            "<span  font_desc='32.5' weight='bold' foreground='black'>Take a break!!!</span>");

    time_label = gtk_label_new("");
    f_set_time_label("");

    GdkColor color;
    gdk_color_parse("red", &color);

    f_update_bg();

    //gtk_signal_connect(GTK_OBJECT (f_window), "key_press_event", GTK_SIGNAL_FUNC (key_press), G_OBJECT(f_window));

    gtk_box_pack_start(GTK_BOX(box), line1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), text, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), time_label, FALSE, FALSE, 0);

    //gtk_box_pack_start(GTK_BOX(box), exit, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(box), line2, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER (f_window), box);

    //gtk_widget_show_all(window);

    //gtk_signal_connect_object(GTK_OBJECT (f_window), "event", GTK_SIGNAL_FUNC (event), G_OBJECT(f_window));

    double x, y;


}

void f_update_bg() {
    GdkPixbuf *image = get_screenshot();
    colorinvert_picture(image);
    GdkPixmap *background = NULL;
    gdk_pixbuf_render_pixmap_and_mask(image, &background, NULL, 0);

    GtkStyle *style = gtk_style_new();
    style->bg_pixmap[0] = background;
    gtk_widget_set_style(GTK_WIDGET(f_window), GTK_STYLE (style) );

}

void f_update_bg_clean() {
    GdkPixbuf *image = get_screenshot();
    GdkPixmap *background = NULL;
    gdk_pixbuf_render_pixmap_and_mask(image, &background, NULL, 0);

    GtkStyle *style = gtk_style_new();
    style->bg_pixmap[0] = background;
    gtk_widget_set_style(GTK_WIDGET(f_window), GTK_STYLE (style) );

}



void f_set_time_label(char *str) {
    gchar *res = g_strdup_printf("<span  font_desc='32.5' weight='bold' foreground='black'>%s</span>", str);
    gtk_label_set_markup(time_label, res);
}

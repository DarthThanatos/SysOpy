#include <gtk/gtk.h>

static void button_clicked(GtkWidget* widget, void * args){
	g_print("Button is pressed \n");
}

int main(int argc, char* argv[]){
	GtkWidget* window, *label, *button, *hbox;
	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	label = gtk_label_new("hello world");
	hbox = gtk_hbox_new(0, 10);
	button = gtk_button_new_with_label("Click me");
	gtk_label_set_text(GTK_LABEL(label),"HELLO");
	gtk_box_pack_start(GTK_BOX(hbox),label, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(hbox),button, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(window),hbox);
	gtk_window_set_title(GTK_WINDOW(window), "GTK+");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_widget_set_size_request(window, 300,300);
	gtk_widget_show_all(window);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(button, "clicked", G_CALLBACK(button_clicked), NULL);
	gtk_main();
	return 0;
}

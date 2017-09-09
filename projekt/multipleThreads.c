#include <unistd.h>
#include <gtk/gtk.h>

#define YES_IT_IS   (1)
#define NO_IT_IS_NOT (0)

char prevText[500];

void
insert_at_handler (GObject *gobject,
               GParamSpec       *pspec,
               GtkTextView *text_area){
	int value, chars_amount;
	printf("inside\n");
	GtkTextIter start,end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(text_area));
	gtk_text_buffer_get_start_iter(buffer,&start); 
	gtk_text_buffer_get_end_iter(buffer,&end); 
	char *text = gtk_text_buffer_get_text(buffer, &start, &end,FALSE);
	g_object_get(gobject,"cursor-position",&value,NULL);
	chars_amount = gtk_text_buffer_get_char_count (buffer);
	g_print("%s insert: %d in buffer: %d\n",text, value, chars_amount);
}

typedef struct
{
  GtkWidget *label;
  int what;
} yes_or_no_args;

G_LOCK_DEFINE_STATIC (yes_or_no);
static volatile int yes_or_no = YES_IT_IS;

void destroy(GtkWidget *widget, gpointer data)
{
  gtk_main_quit();
}

void *argument_thread(void *args)
{
  yes_or_no_args *data = (yes_or_no_args *)args;
  gboolean say_something;

  for(;;)
    {
      /* sleep a while */
      sleep(g_random_int_range (1, 4));

      /* lock the yes_or_no_variable */
      G_LOCK(yes_or_no);

      /* do we have to say something? */
      say_something = (yes_or_no != data->what);

      if(say_something)
	{
	  /* set the variable */
	  yes_or_no = data->what;
	}

      /* Unlock the yes_or_no variable */
      G_UNLOCK(yes_or_no);

      if(say_something)
	{
	  /* get GTK thread lock */
	  gdk_threads_enter();

	  /* set label text */
	  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(data->label));
	  if(data->what == YES_IT_IS)
	    gtk_text_buffer_set_text(buffer, "O yes it is it", 13);
	    //gtk_label_set_text(GTK_LABEL(data->label), "O yes, it is!");
	  else
	    gtk_text_buffer_set_text(buffer, "O no it is no", 13);
	    //gtk_label_set_text(GTK_LABEL(data->label), "O no, it isn't!");

	  /* Make sure all X commands are sent to the X server; not strictly
	   * necessary here, but always a good idea when you do anything
	   * from a thread other than the one where the main loop is running.
	   */
	  gdk_flush ();

	  /* release GTK thread lock */
	  gdk_threads_leave();
	}
    }

  return NULL;
}

int main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *label;
  GError *error = NULL;
  yes_or_no_args yes_args, no_args;

  /* init threads */
  g_thread_init(NULL);
  gdk_threads_init();

  /* init gtk */
  gtk_init(&argc, &argv);

  /* create a window */
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  g_signal_connect(window, "destroy",
		   G_CALLBACK(destroy), NULL);

  gtk_container_set_border_width(GTK_CONTAINER (window), 10);

  /* create a label */
  label = gtk_text_view_new();
  gtk_container_add(GTK_CONTAINER(window), label);

  /* show everything */
  gtk_widget_show(label);
  gtk_widget_show (window);

  /* create the threads */
  yes_args.label = label;
  yes_args.what = YES_IT_IS;
  if (!g_thread_create(argument_thread, &yes_args, FALSE, &error))
    {
      g_printerr ("Failed to create YES thread: %s\n", error->message);
      return 1;
    }

  no_args.label = label;
  no_args.what = NO_IT_IS_NOT;
  if (!g_thread_create(argument_thread, &no_args, FALSE, &error))
    {
      g_printerr ("Failed to create NO thread: %s\n", error->message);
      return 1;
    } 
  g_signal_connect((GTK_TEXT_VIEW(label))->buffer, "notify::cursor-position", G_CALLBACK(insert_at_handler),GTK_TEXT_VIEW(label));


  /* enter the GTK main loop */
  gdk_threads_enter();
  gtk_main();
  gdk_threads_leave();

  return 0;
}
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "common.h"
#include <arpa/inet.h>
#include <sys/select.h>
#include <signal.h>
#include <pthread.h>

char *client_id;
int ip_address, fd_skt;  
struct sockaddr_in net;
struct sockaddr_in myaddr_net;
bool shouldContinue = true;
struct ClientMsg clientMsg;
struct ServerMsg serverMsg;   
fd_set read_set,set;
socklen_t sa_len;
GtkWidget* text_area;
gchar prevText[FILE_CAPACITY];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void changeTextArea(){
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(text_area));
	gtk_text_buffer_set_text(buffer, serverMsg.msg, serverMsg.charactersInMsg);
	memset(prevText, ' ', FILE_CAPACITY);
	strcpy(prevText, serverMsg.msg);
	printf("in change: %s\n", prevText);
}

static void logout_actions(GtkWidget* widget, void * args){
	g_print("Sent request to logout\n");
	strcpy(clientMsg.typeOfMsg,"exit");
	CHECK( sendto(fd_skt, (void *)&clientMsg, sizeof(clientMsg), 0,  (struct sockaddr*) &net, sizeof(net) )!= -1); 
	exit(0);
}

void
state_changed_handler (GtkTextBuffer *textbuffer,
               gpointer       user_data){
	GtkTextIter start,end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(text_area));
	gtk_text_buffer_get_start_iter(buffer,&start); 
	gtk_text_buffer_get_end_iter(buffer,&end); 
	char *text = gtk_text_buffer_get_text(buffer, &start, &end,FALSE);
	int chars_amount = gtk_text_buffer_get_char_count (buffer);
	if (strcmp(prevText,text) != 0 && chars_amount != 0){
		printf("diferrence: %s %s\n", prevText, text);
		strcpy(prevText,text);
		strcpy(clientMsg.typeOfMsg, "setMsg");
		strcpy(clientMsg.msg, text);
		clientMsg.charactersInMsg = gtk_text_buffer_get_char_count (buffer);
		CHECK( sendto(fd_skt, (void *)&clientMsg, sizeof(clientMsg), 0,  (struct sockaddr*) &net, sizeof(net) )!= -1); 
	}
	g_print("%s in buffer: %d\n",text, chars_amount);
}

void
insert_at_handler (GObject *gobject,
               GParamSpec       *pspec,
               GtkTextView *text_area){
	int value;
	GtkTextIter start,end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(text_area));
	gtk_text_buffer_get_start_iter(buffer,&start); 
	gtk_text_buffer_get_end_iter(buffer,&end); 
	char *text = gtk_text_buffer_get_text(buffer, &start, &end,FALSE);
	int chars_amount = gtk_text_buffer_get_char_count (buffer);
	if (strcmp(prevText,text) != 0 && chars_amount != 0){//
		printf("diferrence: %s %s\n", prevText, text);
		strcpy(prevText,text);
		strcpy(clientMsg.typeOfMsg, "setMsg");
		strcpy(clientMsg.msg, text);
		clientMsg.charactersInMsg = gtk_text_buffer_get_char_count (buffer);
		CHECK( sendto(fd_skt, (void *)&clientMsg, sizeof(clientMsg), 0,  (struct sockaddr*) &net, sizeof(net) )!= -1); 
	}
	g_object_get(gobject,"cursor-position",&value,NULL);
	g_print("%s insert: %d in buffer: %d\n",text, value, chars_amount);
}

void handler(int signo){
	if(signo == SIGINT){
		logout_actions(NULL,NULL);
	}
	if(signo == SIGUSR1){
		printf("got usr1\n");
	}
}

void *thread_IO(void * parent_id){
	printf("thread created \n");
	while(true){
		read_set = set;
		select(fd_skt + 1,&read_set,NULL,NULL,NULL);
		if(FD_ISSET(fd_skt,&read_set)){
			int len;
			recvfrom(fd_skt, (void *)&serverMsg, sizeof(serverMsg), 0,(struct sockaddr *)&net, &sa_len);
			printf("Got \"%s\" %d\n", serverMsg.msg, serverMsg.charactersInMsg);  
			//pthread_kill((pthread_t)parent_id, SIGUSR1);
			gdk_threads_enter();
			changeTextArea();
			gdk_flush();
			gdk_threads_leave();
		}
	}
	return NULL;
}

void establishMyPort(){
	//this function increments port number of a client in case 
	//it is already in use
	int err;
	while(true){
		err = bind(fd_skt, (struct sockaddr*) &myaddr_net,  sizeof(myaddr_net));
		if(err == -1){
			if (errno == EADDRINUSE){
				//expected situation
				myaddr_net.sin_port ++;
				printf("once more, port: %d", myaddr_net.sin_port);
			}
			else {
				//another unfortunate event has happend...
				fprintf(stderr,"%s %s:%d: ",__FILE__, __func__, __LINE__); 
				perror("bind: "); 
				exit(-1); 
			}
		}
		else break;
	}
}


void init(int argc, char* argv[]){
	if (argc != 2){
		printf("Wrong usage ./client client_id \n");
		exit(1);
	}
	signal(SIGINT,handler);
	signal(SIGUSR1,handler);
	client_id = argv[1];
	memset(&net, 0, sizeof net);
	CHECK((net.sin_port = htons(SOCKET_PORT)) != -1); 
	net.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	net.sin_family = AF_INET;
	myaddr_net.sin_family = AF_INET;
	myaddr_net.sin_addr.s_addr = inet_addr("127.0.0.1");
	myaddr_net.sin_port = htons(SOCKET_PORT + 1);
	strcpy(clientMsg.client_id, client_id);
	strcpy(clientMsg.typeOfMsg, "register");
	
	CHECK((fd_skt = socket(AF_INET, SOCK_DGRAM, 0)) != -1);  
	struct sockaddr *abstract = (struct sockaddr *)&net;
	establishMyPort();
	g_thread_create(thread_IO,(void *)pthread_self(),FALSE, NULL);
	FD_ZERO(&set);
	FD_SET(fd_skt, &set);
	CHECK( sendto(fd_skt, (void *)&clientMsg, sizeof(clientMsg), 0,  abstract, sizeof(*abstract) )!= -1); 
	printf("sending %d\n", fd_skt);
}
void gtk_exit_ignore(void *data){
	printf("exit...\n");
}

int main(int argc, char* argv[]){
	GtkWidget* window, *scrolled_window, *label, *button, *hbox, *vbox, *name_label;
	g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	label = gtk_label_new("Please type the text in the text area below");
	button = gtk_button_new_with_label("Log out");
	text_area = gtk_text_view_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolled_window), 
                                  GTK_POLICY_AUTOMATIC, 
                                  GTK_POLICY_AUTOMATIC); 
	gtk_container_add (GTK_CONTAINER (scrolled_window), 
                                         text_area);
	hbox = gtk_hbox_new(0, 10);
	vbox = gtk_vbox_new(0,10);
	init(argc, argv);
	name_label = gtk_label_new(clientMsg.client_id);
	gtk_box_pack_start(GTK_BOX(hbox),label, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(hbox),button, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox,0,0,0);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_window,0,0,0);
	gtk_box_pack_start(GTK_BOX(vbox), name_label,0,0,0);
	gtk_container_add(GTK_CONTAINER(window),vbox);
	gtk_window_set_title(GTK_WINDOW(window), "Sys Opy");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_widget_set_size_request(window, 400,315);
	gtk_widget_set_size_request(text_area,400,250);
	gtk_widget_show_all(window);
	g_signal_connect(button, "clicked", G_CALLBACK(logout_actions), NULL);
	g_signal_connect (window, "delete_event", G_CALLBACK (gtk_exit_ignore), NULL);
	//g_signal_connect((GTK_TEXT_VIEW(text_area))->buffer, "notify::cursor-position", G_CALLBACK(insert_at_handler),GTK_TEXT_VIEW(text_area));
	g_signal_connect((GTK_TEXT_VIEW(text_area))->buffer, "changed", G_CALLBACK(state_changed_handler),GTK_TEXT_VIEW(text_area));
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	return 0;
}

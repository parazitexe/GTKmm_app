#include <glib.h>
#include <glib/gprintf.h>

#include <gtkmm.h>

#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>

#include <gst/rtsp/gstrtspurl.h>
#include <gst/rtsp/gstrtspconnection.h>

#include <thread>
#include "AppServer.cpp"



#include <gdk/gdk.h>
#if defined (GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#elif defined (GDK_WINDOWING_WIN32)
#include <gdk/gdkwin32.h>
#elif defined (GDK_WINDOWING_QUARTZ)
#include <gdk/gdkquartz.h>
#endif



#include <stdio.h>

class App : public Gtk::Application
{
protected:

	App() {
		printf("App Constructor \n");
		
		app_timeout = 2;
		path =  "rtsp://admin:admin1@192.168.100.222:554/cam/realmonitor?channel=1&subtype=1";
		
		//create instanse of slass AppServer and put it to pointer named app_server
		//app_server = new AppServer(	app_timeout, path);
		
		//run server to connect to RTSP and stream to localhost in new thread 
		//app_server_thread = new std::thread( [this] { app_server->run_server(); } );
		
	};
	~App(){
		//run app_server Destructor
		//app_server_thread->join();
		//app_server->~AppServer();
		
		printf("App Destructor \n");
	};



protected:
	//Overrides of default signal handlers:
	void on_startup() override{
		//init native "on_startup" function
		Gtk::Application::on_startup();
		
		//create builder and show main ui window that goes from glade
		//builder = Gtk::Builder::create_from_file("window.glade");
		create_and_show_window();
	};
	void on_activate() override{
		//init native "on_activate" function
		Gtk::Application::on_activate();
		
		//get_controls();
		//get_video_window();
		//connect_to_rtsp_and_init_pipeline();
		//play();
		//g_timeout_add_seconds (app_timeout, (GSourceFunc)check_local_status, this);
	};

private:

	void create_and_show_window(){
		builder->get_widget("window1", layouts.app_window);
		add_window(*layouts.app_window);
		layouts.app_window->show_all();
	};

	enum connection_status{
		OFFLINE,
		ONLINE
	};
	
	enum pipeline_status{
		STOPED,
		PAUSED,
		PLAYING_ONLINE
	};

	struct _pipelines{
		gint	connection;
		
		GstElement 	*pipeline_1;
		GstBus 		*bus_1;
		gint	status_pipeline_1;
		
	}pipelines;
	
	struct _layouts{
		Gtk::Window* app_window;
		
		Gtk::DrawingArea* video_window;
	}layouts;
	
	struct _controls{
		//buttons
		Gtk::ToolButton *button_play;
		Gtk::ToolButton *button_pause;
		
	}controls;
	
	Glib::RefPtr<Gtk::Builder> builder;
	gint app_timeout;
	char *path;
	
	
	AppServer *app_server;
	std::thread* app_server_thread;
	
	
public:

	//function for create instanse of App manually from outside
	static Glib::RefPtr<App> create(){
		return Glib::RefPtr<App>(new App());
	};
	
private:
	//get controls from glade and add handler function
	void get_controls(){
		
		builder->get_widget("toolbuttonPlay", controls.button_play);
		controls.button_play->set_sensitive(FALSE); //remove it if you need active button on startup
		controls.button_play->signal_clicked().connect( sigc::mem_fun(*this, &App::play) );
		
		builder->get_widget("toolbuttonPause", controls.button_pause);
		controls.button_pause->set_sensitive(FALSE); //remove it if you need active button on startup
		controls.button_pause->signal_clicked().connect( sigc::mem_fun(*this, &App::pause) );
		
	}
	
	//get video_window from glade
	void get_video_window(){
		
		builder->get_widget("drawingarea1", layouts.video_window);
		
	}
	
	//connect to rtsp and init pipeline
	void connect_to_rtsp_and_init_pipeline(){
		g_message ("connect_to_rtsp_and_init_pipeline \t RUN");
				
		//pipelines.pipeline_1 = gst_parse_launch ("rtspsrc protocols=tcp location=rtsp://admin:admin1@192.168.100.222:554/cam/realmonitor?channel=1&subtype=1 ! rtph264depay ! avdec_h264 ! glimagesink sync=false", NULL);
		pipelines.pipeline_1 = gst_parse_launch ("rtspsrc location=rtsp://127.0.0.1:8554/local_rtsp_server ! rtph264depay ! avdec_h264 ! glimagesink sync=false", NULL);
		
		Glib::RefPtr<Gdk::Window> window = layouts.video_window->get_window();
		
		guintptr window_handle = GDK_WINDOW_XID (window -> gobj());
	
		// Pass it to playbin, which implements VideoOverlay and will forward it to the video sink
		GstElement *sink = gst_bin_get_by_name (GST_BIN (pipelines.pipeline_1), "sink");
		gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (sink), window_handle);
		
		//add buss for handle message
		pipelines.bus_1 = gst_pipeline_get_bus (GST_PIPELINE (pipelines.pipeline_1));
		gst_bus_add_watch (pipelines.bus_1, my_bus_callback, NULL);
		
		
		//init status of pipeline_1
		pipelines.status_pipeline_1 = STOPED;
		

	}
	
	//function for controls
	void play(){
		printf("play emit \n");
		gst_element_set_state (pipelines.pipeline_1, GST_STATE_PLAYING);
		pipelines.status_pipeline_1 = PLAYING_ONLINE;
	}
	void pause(){
		printf("pause emit \n");
		gst_element_set_state (pipelines.pipeline_1, GST_STATE_PAUSED);
		pipelines.status_pipeline_1 = PAUSED;
	}
	void stop(){
		printf("stop emit \n");
		//gst_element_send_event(pipelines.pipeline_1, gst_event_new_eos());
		gst_element_set_state (pipelines.pipeline_1, GST_STATE_NULL);
		pipelines.status_pipeline_1 = STOPED;
	}
	
	//function for sending user-application message to buss 
	// not using right now
	void send_message(){
		g_message("run->send_message");
		gst_element_post_message (pipelines.pipeline_1, gst_message_new_application (GST_OBJECT (pipelines.pipeline_1),  gst_structure_new_empty ("message-test")));
	}
	
	//function for check conection to local server status, stop and resume pipeline
	static gboolean check_local_status ( gpointer data ) {
		printf(" \n--- client message ---  \n");
		printf("check_connection to local RTSP server \n");
		App* instance = (App*)data;
		
		
		//add connection handling strongly need at refactoring move to method
								 
		const gchar 		*urlstr = "rtsp://127.0.0.1:8554/local_rtsp_server";
		
		GstRTSPUrl 			*url;
		GstRTSPConnection 	*conn;
		GTimeVal 			timeout;
		GstRTSPMessage 		*response;
		
		timeout.tv_sec = 	instance->app_timeout;
		timeout.tv_usec = 	0;
		

		gst_rtsp_url_parse( urlstr, &url );
		if(gst_rtsp_connection_create (url, &conn) == GST_RTSP_OK){
			g_message ("gst_rtsp_connection_create \t\t\t DONE");
		}else{
			g_message ("gst_rtsp_connection_create \t\t\t Error");
		}
		if(gst_rtsp_connection_connect_with_response (conn, &timeout, response) == GST_RTSP_OK){
			g_message ("gst_rtsp_connection_connect_with_response \t DONE");
					
			gst_rtsp_connection_close(conn);
			instance->pipelines.connection = ONLINE;
		}else{
			g_message ("gst_rtsp_connection_connect_with_response \t Error");
			instance->pipelines.connection = OFFLINE;
			instance->stop();
		}
		
		printf ("client conection_status - \t %s \n",	(instance->pipelines.connection == ONLINE) ? "ONLINE" : "OFFLINE" );
		
		printf ("client pipeline_status -  \t %s \n", 	(instance->pipelines.status_pipeline_1 == STOPED) ? "STOPED" : ((instance->pipelines.status_pipeline_1 == PAUSED) ? "PAUSED" : "PLAYING_ONLINE") );
		
		if(instance->pipelines.connection == ONLINE && instance->pipelines.status_pipeline_1 == STOPED){
			instance->connect_to_rtsp_and_init_pipeline();
			instance->play();
		}
		
		//send message to buss
		// not using right now
		//instance->send_message();
		
		return TRUE;
	}
	
	//function for handle bus incoming message 
	static gboolean my_bus_callback (GstBus *bus, GstMessage *message, gpointer data) {

	char date_out[25]; 
	time_t date_temp;
	struct tm *date_format;
	time(&date_temp);
	date_format = localtime(&date_temp);
	strftime(date_out, 25, "%Y-%m-%d_%H-%M-%S", date_format);
		
		
		
	switch (GST_MESSAGE_TYPE (message)) {
		case GST_MESSAGE_STREAM_STATUS:{
			const GValue *status_val =  gst_message_get_stream_status_object (message);
			printf ("gst_message_type_get_name %s %s \n", gst_message_type_get_name(message->type), date_out);
			printf ("object: type %s, value %p %s \n", G_VALUE_TYPE_NAME (status_val), g_value_get_object (status_val), date_out);
			break;
		}
		case GST_MESSAGE_EOS:{

			g_message ("GST_MESSAGE_EOSs");
			break;
		}
		default:{
			//printf("gst_message_type_get_name %s %s \n", gst_message_type_get_name(message->type), date_out);
		}
			
	}

		return TRUE;
	}
	

};

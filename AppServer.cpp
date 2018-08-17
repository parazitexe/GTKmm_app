#include <stdio.h>
#include <gst/rtsp-server/rtsp-server.h>

class AppServer{
public:
	AppServer(gint timeout, char *my_path){
		printf("AppServer Constructor \n");
		
		app_server_timeout = timeout;
		path = my_path;
		sprintf(factory_launch_string, "( rtspsrc protocols=tcp location=%s%s", path, " ! rtph264depay  ! rtph264pay pt=96 name=pay0 )");
		
	}
	~AppServer(){
		printf("AppServer Destructor \n");
	}
	
	// run local RTSP server that take RTSP stream and restrim it to localhost
	void run_server(){
		printf("\n - AppServer run_server - \n");
		
		//run server 
		server 		= gst_rtsp_server_new ();
		factory 	= gst_rtsp_media_factory_new ();
		mounts 		= gst_rtsp_server_get_mount_points (server);
		
		
		gst_rtsp_server_attach (server, NULL);
		gst_rtsp_media_factory_set_launch (factory, factory_launch_string);
		
		//add factory server mount points
		gst_rtsp_mount_points_add_factory (mounts, "/local_rtsp_server", factory);
		
		//add signal handler to client connect
		g_signal_connect(G_OBJECT(server), "client-connected", (GCallback)connected, NULL);
		
		//set stream status STREAM_PLAY
		server_status.stream = STREAM_PLAY;
		//check connection status each "app_server_timeout" seconds
		g_timeout_add_seconds ( app_server_timeout, (GSourceFunc)check_connection, this);
		
		printf("\n - AppServer launched - \n");
		
		//run server loop
		loop = g_main_loop_new (NULL, FALSE);
		printf("\n - loop created - \n");
		g_main_loop_run (loop);
		printf("\n - loop run - \n");

		
	}
	
	//function for stop server
	void stop_server(){
		printf("\n - AppServer stoped - \n");
		gst_rtsp_mount_points_remove_factory(mounts, "/local_rtsp_server");
		g_main_loop_quit (loop);
	}
	
	//handle client connection to local server
	//now used right now
	static gboolean connected(GstRTSPServer *gstrtspserver,  GstRTSPClient *arg1, gpointer user_data){
		return TRUE;
	}
	
	//check connection to RTSP
	static gboolean check_connection(gpointer data){
		printf("\n -- server message -- \n");
		printf("check_connection to main RTSP server \n");
		
		AppServer* instance = (AppServer*)data;
		
		const gchar 		*urlstr = instance->path;
		
		GstRTSPUrl 			*url;
		GstRTSPConnection 	*conn;
		GTimeVal 			timeout;
		GstRTSPMessage 		*response;
		
		timeout.tv_sec = 	instance->app_server_timeout;
		timeout.tv_usec = 	0;
		
		
		gst_rtsp_url_parse( urlstr, &url );
		gst_rtsp_connection_create (url, &conn);
		
		
		if(gst_rtsp_connection_connect_with_response (conn, &timeout, response) == GST_RTSP_OK){
			gst_rtsp_connection_close(conn);
			instance->server_status.connection = ONLINE;
		}else{
			instance->stop_server();
			instance->server_status.connection = OFFLINE;
			instance->server_status.stream = STREAM_STOP;
			
		}
		printf ("server connection \t - %s \n",	(instance->server_status.connection == ONLINE) ? "ONLINE" : "OFFLINE" );
		
		printf ("server stream \t\t - %s \n", 	(instance->server_status.stream == STREAM_STOP) ? "SERVER_STOP" : "SERVER_PLAY" );
		
		
		//if connection exist but server not  run yet or stoped
		if(instance->server_status.connection == ONLINE && instance->server_status.stream == STREAM_STOP){
			instance->run_server();
		}
		
		return TRUE;
	}
	

	enum connection_status{
		OFFLINE,
		ONLINE
	};
	
	enum server_stream_status{
		STREAM_PLAY,
		STREAM_STOP
	};

	struct server_status{
		gint	connection;
		gint	stream;
	} server_status;

	GstRTSPServer *server;
	GMainLoop *loop;
	GstRTSPMediaFactory *factory;
	GstRTSPMountPoints *mounts;
	
	gint app_server_timeout;
	char *path;
	char *factory_launch_string;
	
};

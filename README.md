GTKmm_app

install 	Gstreamer : 	sudo apt-get install libgstreamer1.0-0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools

install GstRTSPServer : 	sudo apt-get install libgstrtspserver-1.0-dev

install			GTK3+ :		sudo apt-get install libgtk-3-dev
			
install			GTKmm :		sudo apt-get install libgtkmm-3.0-dev
			
			
string for build: 	g++ -Wall  -o "%e" "%f" `pkg-config --cflags --libs gstreamer-video-1.0 gstreamer-1.0 gtkmm-3.0 gstreamer-rtsp-server-1.0` -w -lgstrtsp-1.0

			
			

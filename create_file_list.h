
#include <gtk/gtk.h>

#include <gst/gst.h>

#include <stdio.h>
#include <dirent.h> 


//strongly need to refactor in future !!!

typedef struct _ListData{
	//gchar 		*playbin; //will be replased to pointter to playbin
	GstElement 	*playbin2; 
	gchar	 	*folder_path;
	gint	 	*reloading_time;
	GtkWidget 	*main_box;
	GtkWidget 	*play_info_label;
	GtkWidget 	*scrolled_window;
	GtkWidget 	*file_list;
	
	
	GtkWidget *video_window;  	// ---- strongly need remove it from this function when refactoring ----
	GtkWidget *video_window2; 	// ---- strongly need remove it from this function when refactoring ----
	
} ListData;

ListData myData;

//function for sortable list row  by compare string in 1-st column
gint sort_iter_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata){
	
	gint ret = 0;
	gchar *name1, *name2;

	gtk_tree_model_get(model, a, 0, &name1, -1);
	gtk_tree_model_get(model, b, 0, &name2, -1);

	ret = g_utf8_collate(name1, name2);
	g_free(name1);
	g_free(name2);

	return -ret;
}

//create global list
void init_list(GtkWidget *list) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *first_column, *second_column;
	GtkListStore *store;
	
	renderer = 	gtk_cell_renderer_text_new ();
	first_column 	= gtk_tree_view_column_new_with_attributes("Files", renderer, "text", 0, NULL);
	second_column 	= gtk_tree_view_column_new_with_attributes("File info", renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), first_column);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), second_column);
	
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	
	GtkTreeSortable *sortable;
	
	sortable = GTK_TREE_SORTABLE(store);
	
	gtk_tree_sortable_set_sort_func(sortable, 0, sort_iter_compare_func, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(sortable, 0, GTK_SORT_ASCENDING);
	
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(list),  GTK_TREE_MODEL(store));
	g_object_unref(store);
}

//get filenames from folder and put into list
void take_files_from_folder(gchar *path, GtkWidget *list){
	
	DIR *d;
	struct dirent *dir;
	if (d = opendir(path)) {
		while (dir = readdir(d)) {
			if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
				continue;
			}
			add_to_list(list, dir->d_name);
		}
		closedir(d);
	}
}

//put one selected item into list
void add_to_list(GtkWidget *list, gchar *str) {
    
	GtkListStore *store;
	GtkTreeIter iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, str, 1, "Info text", -1);
  
}

//reload list 
gboolean reload_list(ListData *data) {
	GtkListStore *store;

	//start get selection
	//remember selected list item
	
	GtkTreeIter iter;
	GtkTreeModel *model;

	GtkTreePath *list_path;
	gchar *list_path_str;
	list_path_str = "";
	
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->file_list));
	
	if(gtk_tree_selection_get_selected(selection, &model, &iter)){
		
		list_path = gtk_tree_model_get_path ( model , &iter );
		list_path_str = gtk_tree_path_to_string(list_path);
		
		gchar *value;
		gtk_tree_model_get(model, &iter, 0, &value,  -1);
		g_free(value);
		
	}
	//end get selection



	//get list store
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(data->file_list)));
	
	//clear list
	gtk_list_store_clear(store);
	
	//create new list
	take_files_from_folder(data->folder_path, data->file_list);

	//set selection to new list
	if(list_path_str != ""){
		gtk_tree_selection_select_path(selection, gtk_tree_path_new_from_string(list_path_str) );
		
	}
	
	return TRUE;
}

//get filename from list and send it to -->>>> 
void on_clicked(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, ListData *data) {
				   
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *value;
	gchar name_file[1000];

	if (gtk_tree_selection_get_selected( gtk_tree_view_get_selection (tree_view), &model, &iter) ) {


		gtk_tree_model_get(model, &iter, 0, &value,  -1);
		gtk_label_set_text(GTK_LABEL(data->play_info_label), value);
		
		
					//start logic for set playbin to play
					//need to integrale logic here
						
						
					gst_element_set_state (data->playbin2, GST_STATE_NULL);
					
					sprintf(name_file, "playbin uri=file://%s%s", data->folder_path, value);
					
					//printf("\n - name_file - %s \n", name_file);
					
					
					data->playbin2 = gst_parse_launch (name_file, NULL);	
						
						
					gtk_widget_hide(data->video_window);
					gtk_widget_show(data->video_window2);
					GdkWindow *window2 = gtk_widget_get_window (data->video_window2);
					
					guintptr window_handle;
					
					/* Retrieve window handler from GDK */
					#if defined (GDK_WINDOWING_WIN32)
						window_handle = (guintptr)GDK_WINDOW_HWND (window2);
					#elif defined (GDK_WINDOWING_QUARTZ)
						window_handle = gdk_quartz_window_get_nsview (window2);
					#elif defined (GDK_WINDOWING_X11)
						window_handle = GDK_WINDOW_XID (window2);
					#endif
						
					gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (data->playbin2), GDK_WINDOW_XID(window2));
					
					
					gst_element_set_state (data->playbin2, GST_STATE_PLAYING);
						
					//end logic for set playbin to play
					
		
		g_free(value);

	}


}

//main widget function 
GtkWidget* create_file_list(gchar* folder_path, gint* reloading_time, gchar* playbin, GtkWidget *video_window, GtkWidget *video_window2){
	
	
	myData.folder_path = folder_path;
	myData.reloading_time = reloading_time;
	myData.playbin2 = playbin;
	
	myData.video_window = video_window;		// ---- strongly need remove it from this function when refactoring ----
	myData.video_window2 = video_window2;	// ---- strongly need remove it from this function when refactoring ----
	
	myData.main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	
	myData.play_info_label = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(myData.main_box), myData.play_info_label, FALSE, FALSE, 0);
	
	myData.scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(myData.main_box), myData.scrolled_window, TRUE, TRUE, 5);
	
	myData.file_list = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(myData.file_list), TRUE);
	g_signal_connect(myData.file_list, "row-activated", G_CALLBACK(on_clicked), &myData);
	
	gtk_container_add(GTK_CONTAINER(myData.scrolled_window), myData.file_list);	
	
	init_list(myData.file_list);
	
	take_files_from_folder(myData.folder_path, myData.file_list);

	g_timeout_add_seconds (myData.reloading_time, (GSourceFunc)reload_list, &myData);
	
	return myData.main_box;
}


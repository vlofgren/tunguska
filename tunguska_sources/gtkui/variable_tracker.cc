#include <gtk/gtk.h>
#include "machine.h"

enum {
	TR_NAME_COL,
	TR_DEC_VALUE_COL,
	TR_NON_VALUE_COL,
	TR_TYPE_COL,
	TR_MEMPOS_COL,
	TR_MEMPOS_STR_COL,
	TR_N_COLS
};

enum {
	VALTYPE_6,
	VALTYPE_12
};

extern machine* mac;

/* Update the displayed value of all tracked variables */
gboolean update_tracker_values(gpointer data) {
	GtkListStore* ls = GTK_LIST_STORE(data);
	GtkTreeIter iter;
	if(!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(ls), &iter)) return TRUE;

	do {
		int mempos, type;
		gtk_tree_model_get(GTK_TREE_MODEL(ls), &iter, TR_MEMPOS_COL, &mempos, TR_TYPE_COL, &type, -1);
		int value = 0;
		char buffer[16];
		if(type == VALTYPE_6) {
			value = mac->memref(mempos).to_int();
			sprintf(buffer, "%.3X", mac->memref(mempos).nonaryhex());
		} else if(type == VALTYPE_12) {
			value = tryte::word_to_int(mac->memref(mempos), mac->memref(mempos+1));
			sprintf(buffer, "%.3X:%.3X", mac->memref(mempos).nonaryhex(),
						     mac->memref(mempos+1).nonaryhex());
		}
		gtk_list_store_set(ls, &iter, TR_DEC_VALUE_COL, value, TR_NON_VALUE_COL, buffer, -1);

	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(ls), &iter));

	return TRUE;

}

/* Add new variable to be tracked */
void track_new_variable(GtkListStore* store, char* name, int mempos, int type) {
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);

	tryte high, low;
	char nonbuffer[16];
	tryte::int_to_word(mempos, high, low);
	sprintf(nonbuffer, "%.3X:%.3X", high.nonaryhex(), low.nonaryhex());

	gtk_list_store_set(store, &iter, TR_NAME_COL, name, 
						 TR_TYPE_COL, type, 
						 TR_MEMPOS_COL, mempos,
						 TR_MEMPOS_STR_COL, nonbuffer, 
						 -1);

}

/* Stop tracking a value */
void stop_tracking(GtkWidget* widget, gpointer data) {
	GtkWidget* tracker = GTK_WIDGET(data);
	GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(tracker));
	GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tracker));
	GtkTreeIter iter;
	if(!gtk_tree_selection_get_selected(selection, &model, &iter)) {
		printf("stop_tracking() unable to find selected variable\n");
		return;
	}
	gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	update_tracker_values(model);
}

/* Set the type of a tracked value to char */
void set_tracked_value_type_char(GtkWidget* widget, gpointer data) {
	GtkWidget* tracker = GTK_WIDGET(data);
	GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(tracker));
	GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tracker));
	GtkTreeIter iter;
	if(!gtk_tree_selection_get_selected(selection, &model, &iter)) return;
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, TR_TYPE_COL, VALTYPE_6, -1);
	update_tracker_values(model);
}

/* Set the type of a tracked value to int */
void set_tracked_value_type_word(GtkWidget* widget, gpointer data) {
	GtkWidget* tracker = GTK_WIDGET(data);
	GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(tracker));
	GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tracker));
	GtkTreeIter iter;
	if(!gtk_tree_selection_get_selected(selection, &model, &iter)) return;
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, TR_TYPE_COL, VALTYPE_12, -1);
	update_tracker_values(model);
}

/* Mouseclick on the tracker */
gboolean tracker_clicked(GtkWidget* tracker, GdkEventButton* evt, gpointer data) {
	/* Only interested in right clicks */
	if(evt->type == GDK_BUTTON_PRESS && evt->button == 3) {

		GtkTreePath* path;
		GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tracker));

		/* Does this click select a row? */
		if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(tracker),
						(gint) evt->x, (gint) evt->y,
						&path, NULL, NULL, NULL)) {
			gtk_tree_selection_unselect_all(selection);
			gtk_tree_selection_select_path(selection, path);
			gtk_tree_path_free(path);

		} else {
			/* If not, try to deselect previously selected row */
			gtk_tree_selection_unselect_all(selection);
		}
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tracker));
		GtkWidget* menu = gtk_menu_new();
		GtkWidget* item = gtk_menu_item_new_with_label("Track new variable");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

		GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(tracker));

		/* Add a remove option to the menu if a row is selected */
		GtkTreeIter iter;
		if(gtk_tree_selection_get_selected(selection, &model , &iter)) {
			item = gtk_menu_item_new_with_label("Remove");
			g_signal_connect(G_OBJECT(item), "activate",
					G_CALLBACK(stop_tracking), tracker);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

			item = gtk_menu_item_new_with_label("Type");
			GtkWidget* submenu = gtk_menu_new();


			gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

			int type;
			gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, TR_TYPE_COL, &type, -1);

			item = gtk_check_menu_item_new_with_label("Character (6t)");
			if(type == VALTYPE_6) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
			else g_signal_connect(G_OBJECT(item), "activate",
					G_CALLBACK(set_tracked_value_type_char), tracker);
			gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);
			item = gtk_check_menu_item_new_with_label("Integer (12t)");
			if(type == VALTYPE_12) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
			else g_signal_connect(G_OBJECT(item), "activate",
					G_CALLBACK(set_tracked_value_type_word), tracker);
			gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);
		}

		gtk_widget_show_all(menu);
		gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, evt->button, gdk_event_get_time((GdkEvent*)(evt)));

		return TRUE;
	} else return FALSE;
}

/* Build variable tracker widget */
GtkWidget* build_tracker() {
	GtkListStore* tracker_store = gtk_list_store_new(TR_N_COLS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING);
	GtkWidget* vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 2);

	track_new_variable(tracker_store, "irq.number", -265720, VALTYPE_6);
	track_new_variable(tracker_store, "irq.data", -265719, VALTYPE_6);
	track_new_variable(tracker_store, "3cc tmp", -264993, VALTYPE_12);
	track_new_variable(tracker_store, "isr.count", 265426, VALTYPE_12);

	GtkWidget* tracker = gtk_tree_view_new_with_model(GTK_TREE_MODEL(tracker_store));

	/* Add cell renderers and columns to tracker */
	GtkCellRenderer* cr = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* col = gtk_tree_view_column_new_with_attributes("Variable", cr, "text", TR_NAME_COL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tracker), col);
	cr = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes("Address", cr, "text", TR_MEMPOS_STR_COL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tracker), col);
	cr = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes("Nonary value", cr, "text", TR_NON_VALUE_COL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tracker), col);
	cr = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes("Decimal value", cr, "text", TR_DEC_VALUE_COL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tracker), col);

	/* Add a scrollbar */
	GtkWidget* scroller = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_container_add(GTK_CONTAINER(scroller), tracker);
//	gtk_widget_set_size_request(scroller, 300, 270);

	/* Hook up some signal handlers */
	/* TODO: This should probably hook to the popup event (or whatever it's called)
	 * as well ... */
	g_signal_connect(G_OBJECT(tracker), "button-press-event",
			G_CALLBACK(tracker_clicked), tracker_store);

	/* Update the tracked values four times per second */
	g_timeout_add(250, &update_tracker_values, tracker_store);

	return scroller;
}


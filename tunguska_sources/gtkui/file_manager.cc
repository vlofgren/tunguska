#include <gtk/gtk.h>
#include <gio/gio.h>

void open_file_by_str(const gchar* file);

enum {
	FM_NAME_COL = 0,
	FM_SHORT_NAME_COL,
	FM_TYPE_COL,
	FM_TYPE_STR_COL,
	FM_MODE_COL,
	FM_N_COLS
};

enum {
	FM_TYPE_3CC,
	FM_TYPE_ASM,
	FM_TYPE_ASM_GENERATED,
	FM_TYPE_TERNOBJ,
	FM_N_TYPES
};

enum {
	FM_MODE_INCLUDE,
	FM_MODE_COMPILE,
	FM_N_MODES
};

const gchar* fm_type_names[FM_N_TYPES] = { ".c", 
					   ".asm",
					   "Generated .asm",
					   ".ternobj" };


const gchar* fm_type_to_name(int mode, int type) {
	const gchar* names[FM_N_MODES][FM_N_TYPES] = {
			{ ".3h", ".asm (incl)", "Generated .asm (incl)", ".ternobj" } ,
			{ ".c",  ".asm", "Generated .asm", "INVALID" }
		};
	return names[mode][type];
}

/* THIS IS JUST TEST CODE --- FOR THE LOVE OF KITTENS, DO NOT USE IT! */
void make_target(const gchar* target, gchar** sources, int source_count) {
	GString* command = g_string_new("tg_assembler");
	command = g_string_append(command, " -o ");
	command = g_string_append(command, target);
	while(source_count--) {
		command = g_string_append_c(command, ' ');
		command = g_string_append(command, sources[source_count]);
	}
	puts(command->str);
	if(!g_spawn_command_line_async(command->str, NULL)) printf("Couldn't spawn :-(\n");

	g_string_free(command, TRUE);
}

/* Walk the build tree and compile in a sensible order */
void build(GtkTreeStore* store, GtkTreeIter* node) {
	if(!node) {
		GtkTreeIter iter;
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
		build(store, &iter);
	} else {
		GtkTreeIter node_copy = *node;
		do {
			gchar* target = NULL;
			gtk_tree_model_get(GTK_TREE_MODEL(store), node, FM_SHORT_NAME_COL, &target, -1);
			int count = 0;	
			GtkTreeIter child;
			if(gtk_tree_model_iter_children(GTK_TREE_MODEL(store), &child, node)) {
				build(store, &child);
				count++;
			}

			gchar** sources = (gchar**) g_malloc(sizeof(gchar*) * (count+2));

			if(gtk_tree_model_iter_children(GTK_TREE_MODEL(store), &child, node)) {
				printf("Building: %s from: ", target);
				int i = 0;
				do {
					int mode;
					int type;
					gchar* source = NULL;
					gtk_tree_model_get(GTK_TREE_MODEL(store), &child, 
							FM_NAME_COL, &source,
							FM_MODE_COL, &mode,
							FM_TYPE_COL, &type, -1);
					sources[i++] = source;
					if(mode == FM_MODE_INCLUDE) {
						printf("(%s) ", source);
					} else {
						printf("%s ", source);
					}
				} while((gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &child)));
				make_target(target, sources, i);
				puts("");
				while(i--) g_free(sources[i]);
				g_free(sources);
			}

			g_free(target);

		} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), node));
	}
}

/* Callbacks */

void fm_row_activated_callback(GtkTreeView* tree_view,
			GtkTreePath* path,
			GtkTreeViewColumn *column,
			gpointer data) {
	GtkTreeIter iter;
	gchar* file = NULL;
	gtk_tree_model_get_iter(gtk_tree_view_get_model(tree_view),
				&iter, path);
	gtk_tree_model_get(gtk_tree_view_get_model(tree_view), &iter, FM_NAME_COL, &file, -1);

	open_file_by_str(file);
	g_free(file);
}

void fm_build_callback(GtkWidget* target,
			gpointer data) {
	build(GTK_TREE_STORE(data), NULL);
	
}

void fm_remove_callback(GtkWidget* target,
			gpointer data) {
	// STUB
	printf("Remove\n");
}

void fm_add_callback(GtkWidget* target,
			gpointer data) {
	// STUB
	printf("Add\n");
}


/* Adding files */

void add_new_file(GtkTreeStore* store, GtkTreeIter* iter, gchar* name, int type, int mode) {
	if(type < 0 || type >= FM_N_TYPES) {
		printf("Attempted to add file with invalid type\n");
	}
	gtk_tree_store_append(store, iter, NULL);
	GFile* file = g_file_new_for_path(name);


	gtk_tree_store_set(store, iter, FM_NAME_COL, name, 
					 FM_SHORT_NAME_COL, g_file_get_basename(file), 
						 FM_TYPE_COL, type,
						 FM_TYPE_STR_COL, fm_type_to_name(mode, type),  
						 FM_MODE_COL, mode,
						 -1);
	g_object_unref(file);
}

void add_new_dependent_file(GtkTreeStore* store, GtkTreeIter* iter, 
			    GtkTreeIter* parent, gchar* name, int type, int mode) {
	if(type < 0 || type >= FM_N_TYPES) {
		printf("Attempted to add file with invalid type\n");
	}
	gtk_tree_store_append(store, iter, parent);
	GFile* file = g_file_new_for_path(name);


	gtk_tree_store_set(store, iter, FM_NAME_COL, name, 
					 FM_SHORT_NAME_COL, g_file_get_basename(file), 
						 FM_TYPE_COL, type,
						 FM_TYPE_STR_COL, fm_type_to_name(mode, type),  
						 FM_MODE_COL, mode,
						 -1);
	g_object_unref(file);
}


GtkWidget* create_file_manager() {
	GtkWidget* box = gtk_vbox_new(FALSE, 2);

	/* Create tree store */
	GtkTreeStore* manager_store = gtk_tree_store_new(FM_N_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);

	GtkTreeIter iter, iter2;

	/* Add some test items */
	add_new_file(manager_store, &iter, "out.ternobj", FM_TYPE_TERNOBJ, FM_MODE_INCLUDE);
	add_new_dependent_file(manager_store, &iter2, &iter, "../../tunguska_memory_image_sources/ram.asm", FM_TYPE_ASM, FM_MODE_COMPILE);
//	add_new_dependent_file(manager_store, &iter2, &iter, "../../tunguska_memory_image_sources///agdp.asm",FM_TYPE_ASM, FM_MODE_INCLUDE);
//	add_new_dependent_file(manager_store, &iter2, &iter, "out.asm", FM_TYPE_ASM_GENERATED, FM_MODE_COMPILE);
//	add_new_dependent_file(manager_store, &iter, &iter2, "../../memory_image_3cc/graphics.c", FM_TYPE_3CC, FM_MODE_COMPILE);
//	add_new_dependent_file(manager_store, &iter, &iter2, "../../memory_image_3cc/system.3h", FM_TYPE_3CC, FM_MODE_INCLUDE);
	build(manager_store, NULL);

	/* Create list view */
	GtkWidget* manager = gtk_tree_view_new_with_model(GTK_TREE_MODEL(manager_store));

	/* Add a handler for doubleclicking on a file */
	g_signal_connect(G_OBJECT(manager), "row-activated",
				G_CALLBACK(fm_row_activated_callback), NULL);

	/* Add columns */
	GtkCellRenderer* cr = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* col = gtk_tree_view_column_new_with_attributes("File", cr, "text", FM_SHORT_NAME_COL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(manager), col);
	cr = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes("Type", cr, "text", FM_TYPE_STR_COL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(manager), col);

	/* Add scrollbars */
	GtkWidget* scroller = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_container_add(GTK_CONTAINER(scroller), manager);


	/* Add toolbar */
	GtkWidget* toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(toolbar), GTK_ICON_SIZE_MENU);

	/* Add "add" button */
	GtkToolItem* tool_button = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(tool_button), "");
	g_signal_connect(G_OBJECT(tool_button), "clicked",
			G_CALLBACK(fm_add_callback), manager_store);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_button, -1);

	/* Add "remove" button */
	tool_button = gtk_tool_button_new_from_stock(GTK_STOCK_CLOSE);
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(tool_button), "");
	g_signal_connect(G_OBJECT(tool_button), "clicked",
			G_CALLBACK(fm_remove_callback), manager_store);
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(tool_button), "");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_button, -1);

	/* Add separator */
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);

	/* Add "build" button */
	tool_button = gtk_tool_button_new_from_stock(GTK_STOCK_EXECUTE);
	g_signal_connect(G_OBJECT(tool_button), "clicked",
			G_CALLBACK(fm_build_callback), manager_store);
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(tool_button), "");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_button, -1);


	/* Add widgets to box */
	gtk_box_pack_start(GTK_BOX(box), scroller, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(box), toolbar, FALSE, FALSE, 2);

	return box;
}

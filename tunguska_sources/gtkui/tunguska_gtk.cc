#include "tunguska_wrapper.h"
#include "../share_dir.h"

#include <gtk/gtk.h>
#include <assert.h>
#include "code_editor.h"
#define VERSION "CVS"


GtkWidget* main_window 	= NULL;
GtkWidget* log 		= NULL;
GtkToolItem* runbutton 	= NULL;
GtkWidget* notebook 	= NULL;

void create_inspector();
void create_procmon();
GtkWidget* create_file_manager();
GtkWidget* build_tracker();


/* * * * * * * * * * * * * * * * * * * * * * * * *
 *						 *
 *						 *
 *						 *
 * File management               		 *
 *						 *
 *						 *
 *						 *
 * * * * * * * * * * * * * * * * * * * * * * * * */

void add_filters_to_file_dialog(GtkFileChooser* dialog) {
	GtkFileFilter* image_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(GTK_FILE_FILTER(image_filter), "Code files (*.c, *.3h, *.asm)");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(image_filter), "*.c");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(image_filter), "*.3h");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(image_filter), "*.asm");
	gtk_file_chooser_add_filter(dialog, image_filter);

	image_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(GTK_FILE_FILTER(image_filter), "All files");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(image_filter), "*");
	gtk_file_chooser_add_filter(dialog, image_filter);
}

CodeEditor* get_current_file() {
	gint page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
	if(page == -1) return NULL;
	GtkWidget* editor = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page);
	if(IS_CODE_EDITOR(editor)) return CODE_EDITOR(editor);
	return NULL;
}

void close_file() {
	gint page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
	if(page == -1) return;
	if(IS_CODE_EDITOR(gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page))) {
		gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), page);
	}
}

void new_file() {
	GtkWidget* editor = code_editor_new();
	GtkWidget* editor_label = code_editor_get_label(editor);
	gtk_widget_show_all(editor);
	gint page = gtk_notebook_append_page(GTK_NOTEBOOK(notebook), editor, editor_label);
	gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(notebook), editor, TRUE);
	if(page != -1) gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), page);
}

void open_file_by_str(const gchar* file) {
	GtkWidget* editor = code_editor_new_from_file(file);
	gtk_widget_show_all(editor);
	if(editor != NULL) {
		GtkWidget* editor_label = code_editor_get_label(editor);
		gint page = gtk_notebook_append_page(GTK_NOTEBOOK(notebook), editor, editor_label);
		gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(notebook), editor, TRUE);
		if(page != -1) gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), page);

	} else {
			// STUB: Here would be a nice place to put an error dialog :-)
			printf("Oops: Editor == NULL\n");
	}

}

void open_file() {
	GtkWidget* dialog = gtk_file_chooser_dialog_new("Open File",
								GTK_WINDOW(main_window),
								GTK_FILE_CHOOSER_ACTION_OPEN,
								GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
								GTK_STOCK_OPEN,	GTK_RESPONSE_ACCEPT,
								NULL);
	ABOUT_TO_OPEN_FILE_DIALOG;
	add_filters_to_file_dialog(GTK_FILE_CHOOSER(dialog));
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		gchar* file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		
		open_file_by_str(file);
		g_free(file);
	}
	gtk_widget_destroy(dialog);
	FILE_DIALOG_NO_LONGER_OPEN;
}

void save_file_as() {
	if(!get_current_file()) return;
	GtkWidget* dialog = gtk_file_chooser_dialog_new("Save File As",
								GTK_WINDOW(main_window),
								GTK_FILE_CHOOSER_ACTION_SAVE,
								GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
								GTK_STOCK_OPEN,	GTK_RESPONSE_ACCEPT,
								NULL);
	add_filters_to_file_dialog(GTK_FILE_CHOOSER(dialog));
	ABOUT_TO_OPEN_FILE_DIALOG;
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char* file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		/*  S T U B S T U B S T U B S T U B
		 *  T U B S T U B S T U B S T U B S
		 *  U B S T U B S T U B S T U B S T
		 *  B S T U B S T U B S T U B S T U
		 *  S T U B S T U B S T U B S T U B
		 *  T U B S T U B S T U B S T U B S
		 *  U B S T U B S T U B S T U B S T
		 *  B S T U B S T U B S T U B S T U
		 *  S T U B S T U B S T U B S T U B
		 *  T U B S T U B S T U B S T U B S
		 *  U B S T U B S T U B S T U B S T
		 *  B S T U B S T U B S T U B S T U
		 *  S T U B S T U B S T U B S T U B
		 *  T U B S T U B S T U B S T U B S
		 *  U B S T U B S T U B S T U B S T
		 *  B S T U B S T U B S T U B S T U
		 */
		g_free(file);
	}
	gtk_widget_destroy(dialog);
	FILE_DIALOG_NO_LONGER_OPEN;
}






/* Create an about dialog */
void about() {
	GtkWidget* about = gtk_about_dialog_new();
	const gchar *authors[] = { "Design and implementation:",
				   "  Viktor Lofgren <vlofgren@gmail.com>",
				   "Porting:",
				   "  Alexander Shabarshin ",
				   NULL};
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about), "Tunguska");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about), "Ternary computer emulator.");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about), VERSION);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about), "http://www.acc.umu.se/~achtt315/tunguska");
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about), authors);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about), "Copyright 2008 Viktor Lofgren");

	gtk_dialog_run(GTK_DIALOG(about));
	gtk_widget_destroy(about);
}


gboolean toggle_run(GtkToggleToolButton* tool, gpointer data) {
	if(gtk_toggle_tool_button_get_active(tool)) {
		run_machine();
		set_keyboard_grabbed(TRUE);
	} else {
		pause_machine();
		set_keyboard_grabbed(FALSE);
	}
	return TRUE;
}

/* Create main window */
void init_ui() {
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(main_window), "Tunguska " VERSION);
	gtk_window_resize(GTK_WINDOW(main_window), 640, 550);
	g_signal_connect(main_window, "destroy",
			G_CALLBACK(gtk_main_quit), NULL);

	GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(main_window), vbox);

	/* Load toolbar and menubar descriptions from an XML file */
	GtkUIManager* manager = gtk_ui_manager_new();
	gtk_ui_manager_add_ui_from_file(manager, SHARE_DIR "/tunguska-gtk-bars.xml", NULL);
	GtkActionEntry ui_actions[] = {
		/* Menubar */
		{ "File", NULL, "_File", NULL, NULL, NULL },
		{ "NewFileAction", GTK_STOCK_NEW, "_New", "<control>N", NULL, new_file },
		{ "OpenFileAction", GTK_STOCK_OPEN, "_Open", NULL, "<Ctrl>O", open_file },
		{ "SaveFileAction", GTK_STOCK_SAVE, "_Save", NULL, "<Ctrl>S", NULL },
		{ "SaveFileAsAction", GTK_STOCK_SAVE_AS, "Save _As", "", NULL, save_file_as },
		{ "CloseAction", GTK_STOCK_CLOSE, "_Close", NULL, "<Ctrl>W", close_file },
		{ "ExitAction", GTK_STOCK_QUIT, "E_xit", NULL, "<Ctrl>Q", gtk_main_quit },
		{ "Edit", NULL, "_Edit", NULL, NULL, NULL },
		{ "UndoAction", GTK_STOCK_UNDO, "_Undo", NULL, "Undo", NULL },
		{ "RedoAction", GTK_STOCK_REDO, "_Redo", NULL, "Redo", NULL },
		{ "CutAction", GTK_STOCK_CUT, "Cu_t", NULL, "Cut", NULL },
		{ "CopyAction", GTK_STOCK_COPY, "_Copy", NULL, "Copy", NULL },
		{ "PasteAction", GTK_STOCK_PASTE, "_Paste", NULL, "Paste", NULL },
		{ "Tunguska", NULL, "_Tunguska", NULL, NULL, NULL },
		{ "LoadImageAction", GTK_STOCK_OPEN, "Load _Image", "", NULL, load_image },
		{ "Disk", NULL, "_Disk", NULL, NULL, NULL },
		{ "LoadDiskAction", GTK_STOCK_OPEN, "_Load Disk", "", NULL, load_disk },
		{ "SaveDiskAction", GTK_STOCK_SAVE_AS, "_Save Disk", "", NULL, save_disk },
		{ "Debug", NULL, "_Debug", NULL, NULL, NULL },
		{ "ProcMonAction", NULL, "_Processor Status", NULL, NULL, create_procmon },
		{ "MemInspectAction", NULL, "_Memory Inspector", NULL, NULL, create_inspector },
		{ "Tracker", NULL, "Tracker", NULL, NULL, NULL },
		{ "AddVarAction", NULL, "Add _Variable", NULL, NULL, NULL },
		{ "Help", NULL, "H_elp", NULL, NULL, NULL },
		{ "AboutAction", GTK_STOCK_ABOUT, "_About", NULL, NULL, about },
		/* Toolbar */
		{ "StepAction", GTK_STOCK_MEDIA_NEXT, "_Step", NULL, "Step one instruction", step_instruction },
		{ "ResetAction", GTK_STOCK_REFRESH, "_Reset", NULL, "Reset Machine", reset_machine },
		{ "InspectorAction", GTK_STOCK_FIND, "_Inspector", NULL, "Memory Inspector", create_inspector },

//		{ "NewFileAction2", GTK_STOCK_NEW, "_New", "<control>N", NULL, new_file },
	};
	GtkToggleActionEntry ui_toggle_actions[] = {
		{ "RunAction", GTK_STOCK_EXECUTE, "_Run", NULL, "Run Machine", NULL, FALSE },		
	};
	GtkActionGroup* actions = gtk_action_group_new("Actions");
	gtk_action_group_add_actions(actions, ui_actions, sizeof(ui_actions) / sizeof(ui_actions[0]), NULL);
	gtk_action_group_add_toggle_actions(actions, ui_toggle_actions, sizeof(ui_toggle_actions) / sizeof(ui_toggle_actions[0]), NULL);
	gtk_ui_manager_insert_action_group(manager, actions, 0);
	GtkAccelGroup* accels = gtk_ui_manager_get_accel_group(manager);
	gtk_window_add_accel_group(GTK_WINDOW(main_window), accels);

	/* Add menu, toolbar to window */
	gtk_box_pack_start(GTK_BOX(vbox), 
		gtk_ui_manager_get_widget(manager, "/ui/MenuBar"), 
		FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), 
		gtk_ui_manager_get_widget(manager, "/ui/ToolBar"), 
		FALSE, TRUE, 0);

	GtkWidget* toolbar = gtk_ui_manager_get_widget(manager, "/ui/ToolBar");
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(toolbar), GTK_ICON_SIZE_SMALL_TOOLBAR);
	/* Ugly hack to add toggle button specific callback to the run button */
	int i;
	for(i = 0; i < 15; i++) {
		if(GTK_IS_TOGGLE_TOOL_BUTTON(gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), i))) {
			runbutton = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), i);
			g_signal_connect(GTK_TOGGLE_TOOL_BUTTON(runbutton), "clicked",
					G_CALLBACK(toggle_run), NULL);
			break;
		}
	}

	notebook = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
	GtkWidget* tunguska_page = gtk_vbox_new(FALSE, 0);

	gtk_box_pack_start(GTK_BOX(tunguska_page), create_wrapper(), FALSE, TRUE, 0);
	
//	gtk_box_pack_start(GTK_BOX(tunguska_page), build_tracker(), TRUE, TRUE, 2);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tunguska_page, gtk_label_new("(Tunguska)"));
	gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(notebook), tunguska_page, TRUE);

	GtkWidget* vpaned = gtk_vpaned_new();
	GtkWidget* hpaned = gtk_hpaned_new();
	GtkWidget* file_mgr = create_file_manager();
	gtk_paned_add1(GTK_PANED(hpaned), file_mgr);
	gtk_paned_add2(GTK_PANED(hpaned), notebook);
	gtk_paned_add1(GTK_PANED(vpaned), hpaned);
	gtk_paned_set_position(GTK_PANED(vpaned), 300);
	gtk_paned_set_position(GTK_PANED(hpaned), 200);
	GtkWidget* notebook2 = gtk_notebook_new();
	log = gtk_text_view_new();

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), log, gtk_label_new("Debug"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), build_tracker(), gtk_label_new("Variable tracker"));
	gtk_paned_add2(GTK_PANED(vpaned), notebook2);
	gtk_box_pack_start(GTK_BOX(vbox), vpaned, TRUE, TRUE, 2);
//	debug_log = gtk_text_view_new();
//	GtkWidget* scroller = gtk_scrolled_window_new(NULL,NULL);
	
//	gtk_container_add(GTK_CONTAINER(scroller), debug_log);
//	gtk_box_pack_start(GTK_BOX(vbox), scroller, TRUE, TRUE, 2);

	/* All done */
	gtk_widget_show_all(main_window);
}


int main(int argc, char* argv[]) {
	gtk_init(&argc, &argv);

	init_ui();
//	init_wrapper();
	gtk_main();
}

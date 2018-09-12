#include "code_editor.h"
#include "../share_dir.h"

#include <glib/gstdio.h>
#include <gio/gio.h>

#ifdef USE_GTKSOURCEVIEW
# include <gtksourceview/gtksourceview.h>
# include <gtksourceview/gtksourcelanguage.h>
# include <gtksourceview/gtksourcelanguagemanager.h>
# include <gtksourceview/gtksourcestyleschememanager.h>
# include <gtksourceview/gtksourceiter.h>
#endif
static gint code_editor_signals[NUMBER_OF_SIGNALS] = { 0, };

void code_editor_set_language(GtkWidget* editor, char* lang);

GType code_editor_get_type() {
	static GType type = 0;
	if(!type) {
		static const GTypeInfo ti = {
			sizeof(struct CodeEditorClass),
			NULL, NULL,
			(GClassInitFunc) code_editor_class_init,		
			NULL, NULL,	
			sizeof(struct CodeEditor),
			0,
			(GInstanceInitFunc) code_editor_init,
		};
		type = g_type_register_static(GTK_TYPE_VBOX,
					"CodeEditor",
					&ti, (GTypeFlags)0);
	}
	return type;
}

void code_editor_class_init(CodeEditorClass* c) {
/*	code_editor_signals[SAVE_SIGNAL] = 
	g_signal_new("save",
		G_TYPE_FROM_CLASS(c),
		(GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET(CodeEditorClass, save),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0); */
}

void code_editor_init(CodeEditor* e) {
	GtkWidget* scroller = gtk_scrolled_window_new(NULL, NULL);

	e->editor = 
#ifdef USE_GTKSOURCEVIEW
	gtk_source_view_new();
#else
	gtk_text_view_new();
#endif	
	PangoFontDescription* fixedwidth = pango_font_description_new();
	pango_font_description_set_family(fixedwidth, "fixed");
	gtk_widget_modify_font(e->editor, fixedwidth);

	gtk_container_add(GTK_CONTAINER(scroller), e->editor);
	gtk_container_add(GTK_CONTAINER(e), scroller);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

#ifdef USE_GTKSOURCEVIEW
	gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(e->editor), TRUE);
	gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(e->editor), TRUE);
#endif
	
	gtk_widget_show_all(scroller);
}

void code_editor_update_name(CodeEditor* e) {
	if(e->filename == NULL) {
		e->shortname = "New File";
	} else {
		GFile* file = g_file_new_for_path(e->filename);
		e->shortname = g_file_get_basename(file);
		g_object_unref(file);
	}
	gtk_label_set_text(GTK_LABEL(e->label), e->shortname);

}

GtkWidget* code_editor_new() {
	GtkWidget* editor = GTK_WIDGET( g_object_new(CODE_EDITOR_TYPE, NULL));
	CODE_EDITOR(editor)->filename = NULL;
	CODE_EDITOR(editor)->shortname = "New File";
	CODE_EDITOR(editor)->label = gtk_label_new("New File");
	return editor;
}

GtkWidget* code_editor_new_from_file(const char* filename) {
	GtkWidget* editor = GTK_WIDGET( g_object_new(CODE_EDITOR_TYPE, NULL));
	CODE_EDITOR(editor)->filename = g_strdup(filename);
	CODE_EDITOR(editor)->label = gtk_label_new("");

	gchar* file_contents = NULL;
	gsize file_size;
	GError* error = NULL;

	if(g_file_get_contents(filename, &file_contents, &file_size, &error)) {
		GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(CODE_EDITOR(editor)->editor));
		gtk_text_buffer_set_text(buffer, file_contents, file_size);
		g_free(file_contents);
	} else {
		printf("%s\n", error->message);
		return NULL;
	}

	code_editor_update_name(CODE_EDITOR(editor));
	code_editor_set_language(editor, "tg_asm");
	return editor;
}

GtkWidget* code_editor_get_label(GtkWidget* editor) {
	CodeEditor* e = CODE_EDITOR(editor);
	return e->label;
}

void code_save(GtkWidget* editor) {

}

void code_editor_save_as(GtkWidget* editor) {

}


void initiate_language_manager() {
	static int once = 0;
#ifdef USE_GTKSOURCEVIEW
	if(!once) {
		GtkSourceLanguageManager* mgr = gtk_source_language_manager_get_default();
		const gchar* const * default_lang_directories = gtk_source_language_manager_get_search_path(mgr);
		int no_language_paths = 0;
		int i;

		/* This isn't very pretty. To add extra search paths, do this: */
		for(; default_lang_directories[no_language_paths] != NULL; no_language_paths++);
		/* (1) Increment this by 1 ----------------------------------------------------v */
		gchar** lang_directories = (gchar**) g_malloc(sizeof(gchar*)*no_language_paths+2);
		for(i = 0; i < no_language_paths; i++) {
			lang_directories[i] = g_strdup(default_lang_directories[i]);
		}
		/* (2) Add another one of this lines: */
		lang_directories[i++] = SHARE_DIR;
		/* Done! */
		lang_directories[i++] = NULL;

		gtk_source_language_manager_set_search_path(mgr, lang_directories);

		for(i = 0; i < no_language_paths; i++) {
			g_free(lang_directories[i]);
		}
		g_free(lang_directories);
		once = 1;
	}
#endif
}

void code_editor_set_language(GtkWidget* editor, char* langid) {
#ifdef USE_GTKSOURCEVIEW
	GtkSourceLanguageManager* mgr = gtk_source_language_manager_get_default();
	initiate_language_manager();
	
	GtkSourceLanguage* language = gtk_source_language_manager_get_language(mgr, langid);
	if(!language) printf("No language\n");

	GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(CODE_EDITOR(editor)->editor));
	gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer), language);
	
	gtk_source_buffer_set_highlight_syntax(GTK_SOURCE_BUFFER(buffer), TRUE);

	GtkSourceStyleSchemeManager* smgr = gtk_source_style_scheme_manager_get_default();
	GtkSourceStyleScheme* scheme = gtk_source_style_scheme_manager_get_scheme(smgr, "classic");
	gtk_source_buffer_set_style_scheme(GTK_SOURCE_BUFFER(buffer), scheme);
#endif
}

#ifndef __CODE_EDITOR_H__
#define __CODE_EDITOR_H__

#include <gtk/gtk.h>
#include <glib-object.h>

#ifdef USE_GTKSOURCEVIEW
# include <gtksourceview/gtksourceview.h>
#endif
G_BEGIN_DECLS

#define CODE_EDITOR_TYPE (code_editor_get_type())
#define CODE_EDITOR(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), CODE_EDITOR_TYPE, CodeEditor))
#define CODE_EDITOR_CLASS(c) (G_TYPE_CHECK_CLASS_CAST((c), CODE_EDITOR_TYPE, CodeEditorClass))
#define IS_CODE_EDITOR(obj)	(G_TYPE_CHECK_INSTANCE_TYPE(obj, CODE_EDITOR_TYPE))
#define IS_CODE_EDITOR_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE(c, CODE_EDITOR_TYPE))
struct CodeEditor {
	GtkVBox parent_data;
	GtkWidget* editor;
	GtkWidget* label;
	const char* filename;
	const char* shortname;
	gboolean changed;
};

typedef struct CodeEditor CodeEditor;

struct CodeEditorClass {
	GtkVBoxClass parent;
//	void (*save) (CodeEditor* e);
};
typedef struct CodeEditorClass CodeEditorClass;


enum {
	SAVE_SIGNAL,
	NUMBER_OF_SIGNALS
};

GType code_editor_get_type();
void code_editor_class_init(CodeEditorClass* c);
void code_editor_init(CodeEditor* e);
GtkWidget* code_editor_new();
GtkWidget* code_editor_new_from_file(const char* filename);
GtkWidget* code_editor_get_label(GtkWidget* editor);
void code_editor_save(GtkWidget* editor);
void code_editor_save_as(GtkWidget* editor);

G_END_DECLS

#endif

#include <gtk/gtk.h>
#include "machine.h"

extern machine* mac;

int start = 0;

gboolean update_inspector(gpointer data) {
	/* Window has been destroyed if the pointer isn't
	 * a text view anymore.
	 *
	 * TODO: Is this the best way of doing this?! */
	if(!GTK_IS_TEXT_VIEW(data)) return FALSE;

	GtkTextView* memory_display = GTK_TEXT_VIEW(data);
	GtkTextBuffer* buffer = gtk_text_view_get_buffer(memory_display);

	char* text = (char*)(g_malloc(729*5));
	int offset = 0;
	tryte high, low;
	for(int r = 0; r < 27; r++) {
		tryte::int_to_word(start+r*27-364, high, low);

		offset += snprintf(text + offset, 729*5 - offset,
			"%.3X:%.3X   "
			"%.3X %.3X %.3X - %.3X %.3X %.3X - %.3X %.3X %.3X | "
			"%.3X %.3X %.3X - %.3X %.3X %.3X - %.3X %.3X %.3X | "
			"%.3X %.3X %.3X - %.3X %.3X %.3X - %.3X %.3X %.3X%c",
				high.nonaryhex(),
				low.nonaryhex(),
				mac->memref(start + r*27 - 364).nonaryhex(),
				mac->memref(start + r*27- 364+1).nonaryhex(),
				mac->memref(start + r*27- 364+2).nonaryhex(),
				mac->memref(start + r*27- 364+3).nonaryhex(),
				mac->memref(start + r*27- 364+4).nonaryhex(),
				mac->memref(start + r*27- 364+5).nonaryhex(),
				mac->memref(start + r*27- 364+6).nonaryhex(),
				mac->memref(start + r*27- 364+7).nonaryhex(),
				mac->memref(start + r*27- 364+8).nonaryhex(),
				mac->memref(start + r*27- 364+9).nonaryhex(),
				mac->memref(start + r*27- 364+10).nonaryhex(),
				mac->memref(start + r*27- 364+11).nonaryhex(),
				mac->memref(start + r*27- 364+12).nonaryhex(),
				mac->memref(start + r*27- 364+13).nonaryhex(),
				mac->memref(start + r*27- 364+14).nonaryhex(),
				mac->memref(start + r*27- 364+15).nonaryhex(),
				mac->memref(start + r*27- 364+16).nonaryhex(),
				mac->memref(start + r*27- 364+17).nonaryhex(),
				mac->memref(start + r*27- 364+18).nonaryhex(),
				mac->memref(start + r*27- 364+19).nonaryhex(),
				mac->memref(start + r*27- 364+20).nonaryhex(),
				mac->memref(start + r*27- 364+21).nonaryhex(),
				mac->memref(start + r*27- 364+22).nonaryhex(),
				mac->memref(start + r*27- 364+23).nonaryhex(),
				mac->memref(start + r*27- 364+24).nonaryhex(),
				mac->memref(start + r*27- 364+25).nonaryhex(),
				mac->memref(start + r*27- 364+26).nonaryhex(),
				r!=26?'\n':' ');
	}


	GtkTextIter begin, end;
	gtk_text_buffer_get_start_iter(buffer, &begin);
	gtk_text_buffer_get_end_iter(buffer, &end);
	char* oldtext = gtk_text_buffer_get_text(buffer, &begin, &end, TRUE);

	/* Only update the text if it has actually changed. (Saving much CPU time and
	 * making it possible to actually select text from the inspector) */
	if(strcmp(text, oldtext) != 0) gtk_text_buffer_set_text(buffer, text, -1);
	g_free(oldtext);

	g_free(text);

	return TRUE;
}

void update_inspector_position(GtkObject* sender, gpointer display) {
	GtkAdjustment* adj = GTK_ADJUSTMENT(sender);
	start = ((int)(gtk_adjustment_get_value(adj)) / 27) * 27;
	update_inspector(display);
}

void create_inspector() {
	GtkWidget* inspector = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(inspector), "Memory Inspector");
	gtk_window_set_resizable(GTK_WINDOW(inspector), FALSE);

	GtkWidget* hbox = gtk_hbox_new(FALSE, 1);
	GtkWidget* memory_display = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(memory_display), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(memory_display), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), memory_display, TRUE, FALSE, 2);

	PangoFontDescription* fixedwidth = pango_font_description_new();
	pango_font_description_set_family(fixedwidth, "fixed");
	gtk_widget_modify_font(memory_display, fixedwidth);

	GtkObject* adj = gtk_adjustment_new(0, -364*729, 364*729, 27, 729, 729);
	GtkWidget* scroller = gtk_vscrollbar_new(GTK_ADJUSTMENT(adj));
	gtk_box_pack_start(GTK_BOX(hbox), scroller, TRUE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(inspector), hbox);
	gtk_widget_show_all(inspector);

	g_timeout_add(100, update_inspector, memory_display);
	g_signal_connect(G_OBJECT(adj), "value-changed", G_CALLBACK(update_inspector_position), memory_display);

	update_inspector(memory_display);

}

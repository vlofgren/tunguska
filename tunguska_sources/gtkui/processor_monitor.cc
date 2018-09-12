#include <gtk/gtk.h>
#include "machine.h"

enum {
	REG_X,
	REG_Y,
	REG_A,
	REG_S,
	REG_PCL,
	REG_PCH,
	REG_PC,
	REG_PG,
	REG_PI,
	REG_PB,
	REG_PV,
	REG_PPR,
	REG_MAX
};

extern machine* mac;

GtkWidget* registers[REG_MAX];

char trit_to_char(int tval) {
	if(tval < 0) return 'N';
	else if(tval == 0) return '0';
	else return 'P';
}
gboolean update_procmon(gpointer data) {
	if(!GTK_IS_WIDGET(registers[0])) return FALSE;

	char buffer[16];
	sprintf(buffer, "%.3X", mac->A.nonaryhex());
	gtk_entry_set_text(GTK_ENTRY(registers[REG_A]), buffer);
	sprintf(buffer, "%.3X", mac->X.nonaryhex());
	gtk_entry_set_text(GTK_ENTRY(registers[REG_X]), buffer);
	sprintf(buffer, "%.3X", mac->Y.nonaryhex());
	gtk_entry_set_text(GTK_ENTRY(registers[REG_Y]), buffer);

	sprintf(buffer, "%.3X", mac->S.nonaryhex());
	gtk_entry_set_text(GTK_ENTRY(registers[REG_S]), buffer);

	sprintf(buffer, "%.3X", mac->PCH.nonaryhex());
	gtk_entry_set_text(GTK_ENTRY(registers[REG_PCH]), buffer);
	sprintf(buffer, "%.3X", mac->PCL.nonaryhex());
	gtk_entry_set_text(GTK_ENTRY(registers[REG_PCL]), buffer);

	sprintf(buffer, "%c", trit_to_char(mac->P[0].to_int()));
	gtk_entry_set_text(GTK_ENTRY(registers[REG_PC]), buffer);
	sprintf(buffer, "%c", trit_to_char(mac->P[1].to_int()));
	gtk_entry_set_text(GTK_ENTRY(registers[REG_PG]), buffer);
	sprintf(buffer, "%c", trit_to_char(mac->P[2].to_int()));
	gtk_entry_set_text(GTK_ENTRY(registers[REG_PI]), buffer);
	sprintf(buffer, "%c", trit_to_char(mac->P[3].to_int()));
	gtk_entry_set_text(GTK_ENTRY(registers[REG_PB]), buffer);
	sprintf(buffer, "%c", trit_to_char(mac->P[4].to_int()));
	gtk_entry_set_text(GTK_ENTRY(registers[REG_PV]), buffer);
	sprintf(buffer, "%c", trit_to_char(mac->P[5].to_int()));
	gtk_entry_set_text(GTK_ENTRY(registers[REG_PPR]), buffer);

	return TRUE;
}

void create_procmon() {
	GtkWidget* procmon = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW(procmon), "Processor monitor");
	gtk_window_set_resizable(GTK_WINDOW(procmon), FALSE);

	PangoFontDescription* fixedwidth = pango_font_description_new();
	pango_font_description_set_family(fixedwidth, "fixed");
	for(int i = 0; i < REG_MAX; i++) {
		registers[i] = gtk_entry_new();
		gtk_entry_set_editable(GTK_ENTRY(registers[i]), FALSE);
		gtk_entry_set_width_chars(GTK_ENTRY(registers[i]), 6);
		gtk_widget_modify_font(registers[i], fixedwidth);
	}

	GtkWidget* vbox = gtk_vbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("General processor registers"), FALSE, TRUE, 0);
	GtkWidget* hbox = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("A"), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), registers[REG_A], FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("S"), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), registers[REG_S], FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("X"), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), registers[REG_X], FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("Y"), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), registers[REG_Y], FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("PCH"), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), registers[REG_PCH], FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("PCL"), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), registers[REG_PCL], FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

	hbox = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Processor status register (P)"), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("C"), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("G"), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("I"), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("B"), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("V"), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("PR"), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), registers[REG_PC], FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), registers[REG_PG], FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), registers[REG_PI], FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), registers[REG_PB], FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), registers[REG_PV], FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), registers[REG_PPR], FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(procmon), vbox);
	g_timeout_add(100, update_procmon, NULL);

	gtk_widget_show_all(procmon);
}

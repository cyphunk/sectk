/***************************************************************************
 *            jtagscan.c
 *
 *  Wed May 31 18:58:11 2006
 *  Copyright  2006  Benedikt 'Hunz' Heinz
 *  jtagscan at hunz.org
 *  $Id: jtagscan.c,v 1.2 2006/05/31 17:20:01 hunz Exp $
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#define SET_TIMER(a)	timer=g_timeout_add((a),timeout,NULL)
#define DEL_TIMER		g_source_remove(timer)

void hexdump(unsigned char *data, int len) {
	int cnt;
	
	for(cnt=0;cnt<len;cnt++)
		g_print("%02x ",data[cnt]);
	g_print("\n");
}

unsigned char txbuf[64];
int txlen;

int debug=0;

time_t starttime;
time_t last_shown;

unsigned long max_comb;
unsigned long done=0;

unsigned char port2cable[32];
unsigned char cable2port[32];

unsigned char shortcuts[32][32];

int ser_fd;
int retries=0;

int startcount;

int last_pin;
int tck=1, tms=2, tdi=3;

GtkTextBuffer *txtbuf;
guint timer=-1;

GladeXML *xml;

unsigned char rxbuf[64];
unsigned char *rxptr=rxbuf;

unsigned short *try2pins=NULL, *pins2try=NULL;

int scan_abort;

void add_text(char *txt) {
	GtkTextIter iter;
	GtkWidget *view = glade_xml_get_widget(xml, "textview1");
	gtk_text_buffer_get_end_iter(txtbuf,&iter);
	gtk_text_buffer_insert(txtbuf,&iter,txt,-1);
	gtk_text_buffer_get_end_iter(txtbuf,&iter);
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(view),&iter, 0, FALSE, 0, 0);
}

gboolean timeout(gpointer data) {
	rxptr=rxbuf;
	//tx();
	add_text("timeout\n");
	return FALSE;
}

void status(char *buf) {
	GtkWidget *w=glade_xml_get_widget(xml,"statusbar1");
	gtk_statusbar_pop(GTK_STATUSBAR(w),0);
	gtk_statusbar_push(GTK_STATUSBAR(w),0,buf);
}

void progress(void) {
	GtkWidget *w=glade_xml_get_widget(xml,"progressbar1");
	gtk_progress_configure(GTK_PROGRESS(w),done,0,max_comb);
}

void tx() {
	
	if(debug>5) {
		g_print("TX: ");
		hexdump(txbuf,txlen);
	}
	
	assert(write(ser_fd,txbuf,txlen)==txlen);
}

int nextpins() {
	char buf[512];
	time_t now=time(NULL);
	int remain, min=0, triespersec;
	GtkWidget *w;
	
	if(++done == max_comb)
		return 0;
	
	if((now > last_shown) && (now > starttime)) {
	
		last_shown=now;
		
		now-=starttime;
	
		triespersec=(done-startcount)/now;
	
		remain=max_comb-done;
		remain/=triespersec;
		
		if(remain>=60) {
			min=1;
			remain/=60;
		}
	
		sprintf(buf,"%ld/%ld done - ETA: %d %s (%d tries/sec.)\n",done,max_comb,remain,min?"min":"sec",triespersec);
		status(buf);
		
		progress();
		
	}
	
	tdi=try2pins[done]&0x1f;
	tms=(try2pins[done]>>5)&0x1f;
	tck=try2pins[done]>>10;
	
	w=glade_xml_get_widget(xml,"tck");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),tck);
	
	w=glade_xml_get_widget(xml,"tms");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),tms);
	
	w=glade_xml_get_widget(xml,"tdi");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(w),tdi);
	
	return 1;
}

void scan_setpins() {
	txbuf[0]=0x80;
	txbuf[0] |= (cable2port[tck-1]<<2);
	txbuf[0] |= (cable2port[tms-1]>>3);
	txbuf[1] = cable2port[tms-1]<<5;
	txbuf[1] |= cable2port[tdi-1];
	txlen=2;
}

gboolean serial_in(GIOChannel *src, GIOCondition cond, gpointer data) {
	int res;
	
	res=read(ser_fd,rxptr,64-(rxptr-rxbuf));
	
	if(debug>5) {
		g_print("RX: ");
		hexdump(rxptr,res);
	}
	
	rxptr+=res;
	assert(rxptr-rxbuf < 64);
	
	/* stage 1: got response to set config */
	if((txbuf[0]==0x40) && (res==1)){
		
		rxptr=rxbuf;
		DEL_TIMER;
		
		/* initiate stage 2: start scan */
		scan_setpins();
		tx();
		SET_TIMER(1000);
	}
	
	/* stage 2: got response to scan cmd */
	else if((txbuf[0]&0x80) && (rxptr-rxbuf == 32)) {
		char buf[5120];
		char tmp[100];
		int tdimatches = rxbuf[cable2port[tdi-1]];
		int print=0;
		
		if (tdimatches != txbuf[2]) {
			sprintf(buf,"weird thing: pattern found %d times on TDI but shifted %d times!??\n",tdimatches,txbuf[2]);
			add_text(buf);
			add_text("this won't work! probably a bug in the microcontroller firmware - check the source!\n");
			scan_abort=1;
		}
		
		/* scan running */
		
		DEL_TIMER;
		
		sprintf(buf,"%d/%d/%d: ",tck,tms,tdi);
		
		if(debug>4)
			g_print(buf,"%d/%d/%d: ",tck,tms,tdi);
		
		for(res=1; res<=last_pin; res++) {

			if(debug>4)
				g_print("%d ",rxbuf[cable2port[res-1]]);

			
			if((res != tck) && (res != tms) && (res != tdi)) {
				int matches=rxbuf[cable2port[res-1]];
				
				if(tdimatches == matches) {
					if(!(shortcuts[tdi][res])) {
						sprintf(tmp,"pin %d and %d seem to be connected! ignoring this... ",tdi,res);
						strcat(buf,tmp);
						shortcuts[tdi][res]=1;
						shortcuts[res][tdi]=1;
						print=1;
					}
				}
				else if((tdimatches > matches) && (matches > 0)) {
					print=1;
					sprintf(tmp,"TDO=%d:%d/%d ",res,matches,tdimatches);
					strcat(buf,tmp);
				}
					
			} // not tdi, tck or tms
		} // foreach pin
		
		if(print) {
			strcat(buf,"\n");
			add_text(buf);
		}
		
		if(debug>4)
			g_print("\n");
		
		rxptr=rxbuf;
		
		if(scan_abort) {

			/* scan aborted or just a single try? */
			if(scan_abort==1)
				add_text("scan aborted\n");
			
			status("");

			return TRUE;
		}
		
		/* combination left? */
		if(nextpins()) {
			scan_setpins();
			tx();
			SET_TIMER(1000);
		}
		else {
			GtkWidget *widget=glade_xml_get_widget(xml, "scan");
			gtk_button_set_label(GTK_BUTTON(widget),"scan");
			status("");
		}
		
	} /* stage 2: got response to a scan cmd */
	
	return TRUE;
}

void table_init(void) {
	int ck, ms, di, cnt=0;
	
	if(!(try2pins)) {
		assert((try2pins=malloc(sizeof(unsigned short)*(32*32*32))));
		assert((pins2try=malloc(sizeof(unsigned short)*(32*32*32))));
	}
	
	for(ck=1;ck<=last_pin; ck++) {
		
		for(ms=1; ms<=last_pin; ms++) {
			
			if(ms==ck)
				continue;
			
			for(di=1; di <= last_pin; di++) {
				
				pins2try[di|(ms<<5)|(ck<<10)]=cnt;
				
				if((ms==di) || (ck==di))
					continue;
				
				try2pins[cnt]=di|(ms<<5)|(ck<<10);
				
				cnt++;
				
			} // tdi
		} // tms
	} // tck
}

void start_scan(int try) {
	GtkWidget *widget;
	const char *pattern;
	char buf[100];
	int cnt;
	
	bzero(shortcuts,sizeof(shortcuts));
	
	assert((widget=glade_xml_get_widget(xml, "ios2")));
	last_pin = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	
	max_comb=last_pin*(last_pin-1)*(last_pin-2);
	
	assert((widget=glade_xml_get_widget(xml, "tck")));
	tck = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	
	if(tck > last_pin)
		last_pin=tck;
	
	assert((widget=glade_xml_get_widget(xml, "tms")));
	tms = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	
	if(tms > last_pin)
		last_pin=tms;
	
	assert((widget=glade_xml_get_widget(xml, "tdi")));
	tdi = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	
	if(tdi > last_pin)
		last_pin=tdi;
	
	assert((widget=glade_xml_get_widget(xml, "ios2")));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),last_pin);
	
	table_init();
	
	startcount=done=pins2try[tdi|(tms<<5)|(tck<<10)];
	
	bzero(txbuf,sizeof(txbuf));
	
	/* pullups? */
	assert((widget=glade_xml_get_widget(xml,"pullups")));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		memset(txbuf+1+1+1+1+4,0xff,4);
	
	txbuf[0]=0x40;
	
	assert((widget=glade_xml_get_widget(xml, "clk")));
	txbuf[1] = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	
	assert((widget=glade_xml_get_widget(xml, "irlen")));
	txbuf[2] = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));

	assert((widget=glade_xml_get_widget(xml,"pattern")));
	pattern=gtk_entry_get_text(GTK_ENTRY(widget));
	
	for(cnt=0;cnt<8;cnt++) {
		txbuf[3]<<=1;
		txbuf[3] |= pattern[cnt]&1;
	}
	
	txlen=1+1+1+1+8;
	
	tx();
	
	retries=0;
	SET_TIMER(1000);
	
	if(!try) {
		sprintf(buf,"starting scan with %ld possible combinations\n",max_comb);
		add_text(buf);
		scan_abort=0;
	}
	else
		scan_abort=2;
	starttime=time(NULL);
}

void on_window1_destroy(GtkWidget *widget, gpointer priv) {
     gtk_main_quit();
}

void on_test_clicked(GtkWidget *widget, gpointer priv) {
	
	if(ser_fd<0)
		return;
	
	start_scan(1);
}

void on_scan_clicked(GtkWidget *widget, gpointer priv) { 

	if(ser_fd<0)
		return;

	if((strcmp(gtk_button_get_label(GTK_BUTTON(widget)),"abort"))) {
		start_scan(0);
		gtk_button_set_label(GTK_BUTTON(widget),"abort");
	}
	else {
		scan_abort=1;
		gtk_button_set_label(GTK_BUTTON(widget),"scan");
	}
}

int load_relocations(char *file) {
	char buf[10];
	int cnt=0;
	FILE *fl=fopen(file,"r");
	
	if(!fl)
		return 0;
	
	while(fgets(buf,5,fl)) {
		int port=atoi(buf);
		port2cable[port]=cnt;
		cable2port[cnt]=port;
		//g_print("%d <-> %d\n",cnt, port);
		cnt++;
	}
	
	fclose(fl);
	
	return 0;
}

void port_init(int fd) {
  struct termios termio;

  tcgetattr(fd,&termio);
  termio.c_cflag = B115200|CS8|CREAD|CLOCAL|PARENB;
  termio.c_iflag = IGNBRK|IGNPAR;
  termio.c_oflag=0;
  termio.c_lflag=0;

  termio.c_cc[VMIN]=1;
  termio.c_cc[VTIME]=100;

  if (tcsetattr(fd,TCSANOW,&termio) < 0) {
    fprintf(stderr,"Error setting COMport-attributes\n");
    _exit(-1);
  }
}

int main(int argc, char **argv) {
	GtkWidget *iface;
	GtkWidget *txt;
	char *ser_device=getenv("JTAGSCAN_DEVICE");
	
	gtk_init(&argc, &argv);
	glade_init();
	
	xml = glade_xml_new("jtagscan.glade", NULL, NULL);
	
	glade_xml_signal_autoconnect(xml);
	
	assert((txtbuf = gtk_text_buffer_new(NULL)));
	
	assert((txt=glade_xml_get_widget(xml, "textview1")));
	
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(txt), txtbuf);
	
	if(!ser_device) {
		fprintf(stderr,"no device given! set JTAGSCAN_DEVICE to the serial port\n");
		return 23;
	}

	ser_fd=open(ser_device,O_RDWR);
	
	iface=glade_xml_get_widget(xml, "statusbar1");
	
	if(getenv("DEBUG"))
		debug=atoi(getenv("DEBUG"));
	else
		debug=0;
	
	add_text("$Id: jtagscan.c,v 1.2 2006/05/31 17:20:01 hunz Exp $\n");
	
	if(ser_fd>=0) {
		port_init(ser_fd);
	
		g_io_add_watch(g_io_channel_unix_new(ser_fd), G_IO_IN, serial_in, NULL);
	
		add_text("connected to ");
		add_text(ser_device);
		add_text(" with 115200 baud\n");
		
		gtk_statusbar_push(GTK_STATUSBAR(iface),0,"connected");
	}
	else {
		add_text("can't open ");
		add_text(ser_device);
		add_text("\n");
		gtk_statusbar_push(GTK_STATUSBAR(iface),0,"NOT connected!!!");
	}
	
	assert(!(load_relocations("relocations.avr")));
	
	add_text("relocations loaded\n");
		
	gtk_main();
	
	return 0;
}

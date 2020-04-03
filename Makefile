src=src
res=res
bin=bin
install_bin=~/.local/bin
autostart_bin=~/.config/autostart
glib=`pkg-config --cflags --libs glib-2.0`
gdk-pixbuf=`pkg-config --cflags --libs gdk-pixbuf-2.0`
flags=-lsystemd -lnotify 
compiler=gcc

output_directory:
	if [ -d ./bin ]; then echo 'Build directory exists'; else mkdir -p $(bin); fi

install:
	cp $(bin)/journal-notifications $(install_bin)/journal-notifications
	cp $(res)/Journal\ Notifications.desktop $(autostart_bin)/Journal\ Notifications.desktop

uninstall:
	rm $(autostart_bin)/Journal\ Notifications.desktop
	rm $(install_bin)/journal-notifications

clean:
	rm $(bin)/*

.DEFAULT_GOAL := default

default: output_directory
	$(compiler) $(flags) $(glib) $(gdk-pixbuf) $(src)/journal-notifications.c -o $(bin)/journal-notifications

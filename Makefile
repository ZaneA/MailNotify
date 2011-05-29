mailnotify : mailnotify.c
	gcc -o mailnotify mailnotify.c `pkg-config --cflags --libs gtk+-2.0`

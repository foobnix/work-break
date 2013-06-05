:: pkg-config --cflags --libs gtk+-win32-2.0
SET VAR=-mms-bitfields -Ic:/Users/iviv/Downloads/gtk+-bundle_2.24.10-20120208_win32/include/gtk-2.0 -Ic:/Users/iviv/Downloads/gtk+-bundle_2.24.10-20120208_win32/lib/gtk-2.0/include -Ic:/Users/iviv/Downloads/gtk+-bundle_2.24.10-20120208_win32/include/atk-1.0 -Ic:/Users/iviv/Downloads/gtk+-bundle_2.24.10-20120208_win32/include/cairo -Ic:/Users/iviv/Downloads/gtk+-bundle_2.24.10-20120208_win32/include/gdk-pixbuf-2.0 -Ic:/Users/iviv/Downloads/gtk+-bundle_2.24.10-20120208_win32/include/pango-1.0 -Ic:/Users/iviv/Downloads/gtk+-bundle_2.24.10-20120208_win32/include/glib-2.0 -Ic:/Users/iviv/Downloads/gtk+-bundle_2.24.10-20120208_win32/lib/glib-2.0/include -Ic:/Users/iviv/Downloads/gtk+-bundle_2.24.10-20120208_win32/include -Ic:/Users/iviv/Downloads/gtk+-bundle_2.24.10-20120208_win32/include/freetype2 -Ic:/Users/iviv/Downloads/gtk+-bundle_2.24.10-20120208_win32/include/libpng14  -Lc:/Users/iviv/Downloads/gtk+-bundle_2.24.10-20120208_win32/lib -lgtk-win32-2.0 -lgdk-win32-2.0 -latk-1.0 -lgio-2.0 -lpangowin32-1.0 -lgdi32 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lpango-1.0 -lcairo -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lglib-2.0 -lintl -pthread  
SET P=C:\Users\iviv\Dropbox\test\src

del timer.exe
gcc %P%\core.c %P%\about.c %P%\fullscreen.c %P%\preferences.c %P%\trayicon.c -I%P% %VAR% -mwindows -o timer.exe
:: gcc %P%\test.c -I%P% %VAR% -mnowindows -o  timer.exe
:: gcc C:\Users\iviv\workspace-new\testc\src\teste.c %VAR% -lgtk-win32-2.0
:: gcc C:\Users\iviv\workspace-new\testc\src\clock.c %VAR% -mwindows -lgtk-win32-2.0
timer.exe


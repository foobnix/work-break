:: pkg-config --cflags --libs gtk+-win32-2.0
:: SET GDIR=win
SET GDIR=c:\Users\iviv\Downloads\gtk+-bundle_2.24.10-20120208_win321
SET VAR=-mms-bitfields -I%GDIR%/include/gtk-2.0 -I%GDIR%/lib/gtk-2.0/include -I%GDIR%/include/atk-1.0 -I%GDIR%/include/cairo -I%GDIR%/include/gdk-pixbuf-2.0 -I%GDIR%/include/pango-1.0 -I%GDIR%/include/glib-2.0 -I%GDIR%/lib/glib-2.0/include -I%GDIR%/include -I%GDIR%/include/freetype2 -I%GDIR%/include/libpng14  -L%GDIR%/lib -lgtk-win32-2.0 -lgdk-win32-2.0 -latk-1.0 -lgio-2.0 -lpangowin32-1.0 -lgdi32 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lpango-1.0 -lcairo -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lglib-2.0 -lintl -pthread  
SET P=C:\Users\iviv\git\work-break\src
SET APP_NAME=work-break.exe
windres icon.rc -O coff -o icon.res

del %APP_NAME%
gcc %P%\core.c %P%\about.c %P%\fullscreen.c %P%\preferences.c %P%\trayicon.c -I%P% %VAR% -mwindows -o %APP_NAME% icon.res
:: gcc %P%\test.c -I%P% %VAR% -mnowindows -o  timer.exe
:: gcc C:\Users\iviv\workspace-new\testc\src\teste.c %VAR% -lgtk-win32-2.0
:: gcc C:\Users\iviv\workspace-new\testc\src\clock.c %VAR% -mwindows -lgtk-win32-2.0
%APP_NAME%


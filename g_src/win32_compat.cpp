#include <string>

#include "enabler.h"
#include "platform.h"
#ifndef WIN32
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <signal.h>
# include <errno.h>
# include <stdio.h>
# include <string.h>
# ifdef __APPLE__
#  include "osx_messagebox.h"
# elif defined(unix)
#  include <gtk/gtk.h>
# endif
#endif

#ifndef WIN32
BOOL CreateDirectory(const char* pathname, void*)
{
  if (mkdir(pathname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
    if (errno != EEXIST) {
      std::string emsg = "mkdir(";
      emsg.append(pathname);
      emsg.append(") failed");
      perror(emsg.c_str());
    }
    return FALSE;
  } else {
    return TRUE;
  }
}

BOOL DeleteFile(const char* filename)
{
  return !unlink(filename);
}

void ZeroMemory(void* dest, int len)
{
  memset(dest, 0, len);
}

/* Returns milliseconds since 1970
 * Wraps every 24 days (assuming 32-bit signed dwords)
 */
DWORD GetTickCount()
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

// Fills performanceCount with microseconds passed since 1970
// Wraps in twenty-nine thousand years or so
BOOL QueryPerformanceCounter(LARGE_INTEGER* performanceCount)
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  performanceCount->QuadPart = ((long long)tp.tv_sec * 1000000) + tp.tv_usec;
  return TRUE;
}

BOOL QueryPerformanceFrequency(LARGE_INTEGER* performanceCount)
{
  /* A constant, 10^6, as we give microseconds since 1970 in
   * QueryPerformanceCounter. */
  performanceCount->QuadPart = 1000000;
  
  return TRUE;
}

int MessageBox(HWND *dummy, const char *text, const char *caption, UINT type)
{
  bool toggle_screen = false;
  int ret = IDOK;
  if (enabler.is_fullscreen()) {
    enabler.toggle_fullscreen();
    toggle_screen = true;
  }
# ifdef __APPLE__ // Cocoa code
  if (type & MB_OK) {
    CocoaAlertPanel(caption, text, "OK", NULL, NULL);
  } else if (type & MB_YESNO) {
    ret = CocoaAlertPanel(caption, text, "Yes", "No", NULL);
    ret = (ret == 0 ? IDNO : IDYES);
  }
# else // GTK code
  GtkWidget *dialog = gtk_message_dialog_new(NULL,
					     GTK_DIALOG_DESTROY_WITH_PARENT,
					     type & MB_YESNO ?
					     GTK_MESSAGE_QUESTION :
					     GTK_MESSAGE_ERROR,
					     type & MB_YESNO ?
					     GTK_BUTTONS_YES_NO :
					     GTK_BUTTONS_OK,
					     "%s", text);
  gtk_window_set_position((GtkWindow*)dialog, GTK_WIN_POS_CENTER_ALWAYS);
  gtk_window_set_title((GtkWindow*)dialog, caption);
  gint dialog_ret = gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
  while (gtk_events_pending())
    gtk_main_iteration();
				       
  if (type & MB_YESNO) {
    switch (dialog_ret) {
    default:
    case GTK_RESPONSE_DELETE_EVENT:
    case GTK_RESPONSE_NO:
      ret = IDNO;
      break;
    case GTK_RESPONSE_YES:
      ret = IDYES;
      break;
    }
  }
# endif
	
  if (toggle_screen) {
    enabler.toggle_fullscreen();
  }
	
  return ret;
}
#endif

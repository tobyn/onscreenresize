#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

int main(int argc, char *argv[]) {
  Display *display;
  Window root_window;

  Bool randr_present;
  int randr_event_base, randr_error_base;

  XEvent event;
  int debounced_events;
  unsigned int debounce_interval;

  char *shell_command;
  int exit_status;

  if (argc < 3) {
    fprintf(stderr,"Usage: %s <debounce-ms> <command>\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  errno = 0;
  debounce_interval = strtoul(argv[1],NULL,10) * 1000;
  if (errno != 0) {
    fprintf(stderr,"Invalid debounce interval: %s\n",argv[1]);
    exit(EXIT_FAILURE);
  }

  shell_command = argv[2];

  display = XOpenDisplay(NULL);
  if (!display) {
    fputs("Unable to open DISPLAY.\n",stderr);
    exit(EXIT_FAILURE);
  }

  randr_present = XRRQueryExtension(
    display,
    &randr_event_base,
    &randr_error_base);

  if (randr_present == False) {
    fputs("This display lacks RandR support.\n",stderr);
    exit(1);
  }

  root_window = RootWindow(display,DefaultScreen(display));
  XRRSelectInput(display,root_window,RRScreenChangeNotifyMask);

  while (1) {
    XNextEvent(display,&event);
    XRRUpdateConfiguration(&event);

    if (event.type - randr_event_base != RRScreenChangeNotify)
      continue;

    if (debounce_interval > 0) {
      usleep(debounce_interval);

      debounced_events = XEventsQueued(display,QueuedAfterReading);
      while (debounced_events-- > 0) {
        XNextEvent(display,&event);
        XRRUpdateConfiguration(&event);
      }
    }

    exit_status = system(shell_command);
    if (exit_status != 0) {
      fprintf(stderr,"%s exited with status %d\n",shell_command,exit_status);
      exit(1);
    }
  }

  /* This never happens, but if we ever got here it should. */
  XCloseDisplay(display);
  return EXIT_SUCCESS;
}

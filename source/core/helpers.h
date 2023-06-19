#ifndef URATOOL_HELPERS_H
#define URATOOL_HELPERS_H

#define FAST_EXIT_ON_ERROR(boolstmt, msg, rtcode) do { if (!boolstmt) \
	{ if (msg != NULL) printf(msg); exit(rtcode); } } while (0)

#define URATOOL_UDEV_UNREF(pointer, func) do { func(pointer); pointer = NULL; } while (0)

#endif

if DEBUG
AM_CFLAGS=-O0 -g -Wall -Wextra -fPIC
else
AM_CFLAGS=-O2 -g -fPIC
endif

AM_CPPFLAGS = -I$(top_srcdir)/include

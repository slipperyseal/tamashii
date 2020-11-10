
-include $(TOPDIR)/Make.defs

# Tamashii Catchpole Robotics Embedded System

CXXSRCS = console.cxx gps.cxx thread.cxx queue.cxx esp.cxx telemetry.cxx geo.cxx
MAINSRC = tamashii_main.cxx

# tamashii built-in application info

PROGNAME = tamashii
PRIORITY = SCHED_PRIORITY_DEFAULT
STACKSIZE = $(CONFIG_DEFAULT_TASK_STACKSIZE)
MODULE = $(CONFIG_CATCHPOLE_TAMASHII)

include $(APPDIR)/Application.mk

#!/usr/bin/python
import dbus, dbus.service, dbus.mainloop.glib
import gobject
from sys import stdout, argv

class Uberprogram(dbus.service.FallbackObject):
    def __init__(self, busname):
        dbus.service.FallbackObject.__init__(self, dbus.SessionBus(), '/')
        self.iam = dbus.service.BusName(busname, dbus.SessionBus())

    @dbus.service.method(dbus_interface='com.nokia.MApplicationIf',
                         in_signature='as')
    def launch(self, uris):
        print 'launch:', uris
        stdout.flush()

    @dbus.service.method(dbus_interface="uber.program")
    def mime_open(self, *args):
        print 'mime_open: ', args
        stdout.flush()

print "started"
stdout.flush()
dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
# if there is a command line parameter, use it as bus name (otherwise, use the
# default bus name)
busname = "uber.program"
if len(argv) >= 2:
    busname = argv[1]
uber = Uberprogram(busname)
gobject.MainLoop().run()

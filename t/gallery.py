#!/usr/bin/python
import dbus, dbus.service, dbus.mainloop.glib
import gobject
from sys import stdout, argv

class Gallery(dbus.service.Object):
    def __init__(self, path, busname):
        dbus.service.Object.__init__(self, dbus.SessionBus(), path)
        self.iam = dbus.service.BusName(busname, dbus.SessionBus())

    @dbus.service.method(dbus_interface='com.nokia.galleryserviceinterface',
                         in_signature='as', out_signature='b')
    def showImage(self, uris):
        print 'showImage ; %s' % (','.join(uris))
        stdout.flush()
        return True

print "started"
stdout.flush()
dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
# if there is a command line parameter, use it as bus name (otherwise, use the
# default bus name)
busname = "just.a.gallery"
if len(argv) >= 2:
    busname = argv[1]
gallery = Gallery('/', busname)
gobject.MainLoop().run()

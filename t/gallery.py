#!/usr/bin/python2.5
import dbus, dbus.service, dbus.mainloop.glib
import gobject
from sys import stdout

class Gallery(dbus.service.Object):
    def __init__(self, path):
        dbus.service.Object.__init__(self, dbus.SessionBus(), path)
        self.iam = dbus.service.BusName('just.a.gallery', dbus.SessionBus())

    @dbus.service.method(dbus_interface='com.nokia.galleryserviceinterface',
                         in_signature='sas', out_signature='b')
    def showImage(self, uri, uris):
        print 'showImage ; %s ; %s' % (uri, ','.join(uris))
        stdout.flush()
        return True

print "started"
stdout.flush()
dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
gallery = Gallery('/')
gobject.MainLoop().run()

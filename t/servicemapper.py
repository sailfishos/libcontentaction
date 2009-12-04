#!/usr/bin/python2.5
import os
import dbus, dbus.service, dbus.mainloop.glib
import gobject

class ServiceMapper(dbus.service.Object):
    def __init__(self, path):
        dbus.service.Object.__init__(self, dbus.SessionBus(), path)
        self.iam = dbus.service.BusName('com.nokia.DuiServiceFw',
                                        dbus.SessionBus())
        self.mapping = {}
        try:
            execfile('service.map')
        except IOError:
            execfile(os.path.dirname(__file__) + '/service.map')

    @dbus.service.method(dbus_interface='com.nokia.DuiServiceFwIf',
                         in_signature='s', out_signature='s')
    def serviceName(self, interface):
        return self.mapping.get(interface, '')

dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
napper = ServiceMapper('/')
gobject.MainLoop().run()

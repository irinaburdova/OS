#! usr/bin/env python

import sys
import getopt
import dbus
import pyinotify
import asyncore

USAGE_MSG = """Usage: notification.py [options]
Options:
-h, --help                     show this help message and exit
-n user_id, --name=user_id     set user id
-r rec_id, --receiver=r_id     set receiver id
-p proto, --protocol=proto     set protocol, example: icq
-o path, --object=path         set object path, example: /home/irin - folder
"""


def print_usage_and_die():
    print(USAGE_MSG)
    sys.exit(2)


def parse_args_or_die(argv):
    name = None
    receiver=None
    protocol = None
    object_path = None

    try:
        opts, args = getopt.getopt(
            argv, 'n:r:p:o:h', ['name=', 'receiver=', 'protocol=', 'object=', 'help'])
    except getopt.GetoptError:
        print_usage_and_die()

    for opt, arg in opts:
        if opt in ('-h', '--help'):
            print_usage_and_die()
            sys.exit(2)
        elif opt in ('-n', '--name'):
            name = arg
        elif opt in ('-r', '--receiver'):
            receiver = arg
        elif opt in ('-p', '--protocol'):
            protocol = arg
        elif opt in ('-o', '--object'):
            object_path = arg
        else:
            print_usage_and_die()

    if name is None or receiver is None or protocol is None or object_path is None:
        print_usage_and_die()

    return name, receiver, protocol, object_path


def get_interface():
    session_bus = dbus.SessionBus()
    purple_obj = session_bus.get_object("im.pidgin.purple.PurpleService",
                                        "/im/pidgin/purple/PurpleObject")
    return dbus.Interface(purple_obj, "im.pidgin.purple.PurpleInterface")


def send_message_for_event(name, receiver, protocol, event, path):
    protocols = {'icq': 'prpl-icq', 'xmpp': 'prpl-jabber', 
                 'jabber': 'prpl-jabber', 'gtalk': 'prpl-jabber'}
    PURPLE_CONV_TYPE_IM = 1
    message = ''
    try:
        session_bus = dbus.SessionBus()
        purple_obj = session_bus.get_object("im.pidgin.purple.PurpleService",
                                        "/im/pidgin/purple/PurpleObject")
    except:
        print "Error: check if Pidgin is on"
        sys.exit(2)
    mask = event.split('|')
    for ev in mask: 
        if ev.startswith('IN_DELETE'):
            message += 'action: Subfile was deleted, path: '+path
        elif ev.startswith('IN_ACCESS'):
            message += 'action: File was accessed, path: '+path
        elif ev.startswith('IN_ATTRIB'):
            message += 'action: Metadata changed, path: '+path
        elif ev.startswith('IN_CLOSE_NOWRITE'):
            message += 'action: Unwrittable file closed, path: '+path
        elif ev.startswith('IN_CLOSE_WRITE'):
            message += 'action: Writtable file was closed, path: '+path
        elif ev.startswith('IN_CREATE'):
            message += 'action: Subfile was created: '+path
        elif ev.startswith('IN_DELETE_SELF'):
            message += 'action: Self (watched item itself) was deleted, path: '+path
        elif ev.startswith('IN_MODIFY'):
            message += 'action: File was modified, path: '+path
        elif ev.startswith('IN_MOVE_SELF'):
            message += 'action: Self (watched item itself) was moved, path: '+path
        elif ev.startswith('IN_OPEN'):
            message += 'action: File was opened, path: '+path

    ac_number = purple_obj.get_dbus_method('PurpleAccountsFind', dbus_interface='im.pidgin.purple.PurpleService')(name, protocols[protocol])
    if not ac_number:
        raise SystemExit('error: no account for {0}'.format(name))
    try:
        conv_new = purple_obj.get_dbus_method('PurpleConversationNew',
                                              dbus_interface='im.pidgin.purple.PurpleService')(PURPLE_CONV_TYPE_IM, ac_number, receiver)
        conv_im = purple_obj.get_dbus_method('PurpleConvIm', dbus_interface='im.pidgin.purple.PurpleService')(conv_new)
        purple_obj.get_dbus_method('PurpleConvImSend', dbus_interface='im.pidgin.purple.PurpleService')(conv_im, message)
    except:
        print "Can't send message"

class EventHandler(pyinotify.ProcessEvent):
    def __init__(self, name, receiver, protocol, object_path):
        pyinotify.ProcessEvent.__init__(self)

        self.name = name
        self.receiver = receiver
        self.protocol = protocol
        self.object_path = object_path

    def process_IN_DELETE(self, event):
        send_message_for_event(self.name, self.receiver, self.protocol, event.maskname,
                               event.pathname)
    def process_IN_ACCESS(self, event):
        send_message_for_event(self.name, self.receiver, self.protocol, event.maskname,
                               event.pathname)
    def process_IN_ATTRIB(self, event):
        send_message_for_event(self.name, self.receiver, self.protocol, event.maskname,
                               event.pathname)
    def process_IN_CLOSE_NOWRITE(self, event):
        send_message_for_event(self.name, self.receiver, self.protocol, event.maskname,
                               event.pathname)
    def process_IN_CLOSE_WRITE(self, event):
        send_message_for_event(self.name, self.receiver, self.protocol, event.maskname,
                               event.pathname)
    def process_IN_CREATE(self, event):
        send_message_for_event(self.name, self.receiver, self.protocol, event.maskname,
                               event.pathname)
    def process_IN_DELETE_SELF(self, event):
        send_message_for_event(self.name, self.receiver, self.protocol, event.maskname,
                               event.pathname)
    def process_IN_MODIFY(self, event):
        send_message_for_event(self.name, self.receiver, self.protocol, event.maskname,
                               event.pathname)
    def process_IN_MOVE_SELF(self, event):
        send_message_for_event(self.name, self.receiver, self.protocol, event.maskname,
                               event.pathname)
    def process_IN_OPEN(self, event):
        send_message_for_event(self.name, self.receiver, self.protocol, event.maskname,
                               event.pathname)

def main(argv):
    name, receiver, protocol, object_path = parse_args_or_die(argv)

    #Adding inotify watch manager for all events
    wm = pyinotify.WatchManager()
    mask = pyinotify.ALL_EVENTS
    pyinotify.AsyncNotifier(wm, EventHandler(name, receiver, protocol, object_path))
    wm.add_watch(object_path, mask, rec=True)
    asyncore.loop()

if __name__ == "__main__":
    main(sys.argv[1:])

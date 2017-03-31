TEMPLATE = aux

include(../../common.pri)

desktop_tests.path = $$CONTENTACTION_TESTDIR/applications
desktop_tests.files = \
    mimeinfo.cache \
    mimeapps.list \
    uberexec.desktop \
    unterexec.desktop \
    ubermimeopen.desktop \
    caller.desktop \
    addcontact.desktop \
    contacthandler.desktop \
    emailer.desktop \
    galleryserviceinterface.desktop \
    fixedparams.desktop \
    plainimageviewer.desktop \
    trackerimageviewer.desktop \
    plainmusicplayer.desktop \
    complexmusic.desktop \
    uriprinter.desktop \
    gallerywithfilename.desktop \
    browser.desktop \
    special-browser.desktop \
    regexpmatcher.desktop 
INSTALLS += desktop_tests

unix{
    other_desktop.target = other.desktop
    other_desktop.files = other.desktop
    other_desktop.path = $$CONTENTACTION_TESTDIR/applications
    other_desktop.CONFIG = no_check_exist
    other_desktop.commands = sed \'s%@PATH@%$${CONTENTACTION_TESTDIR}%\' $$PWD/other.desktop.in > $$PWD/other.desktop

    show_desktop.target = show.desktop
    show_desktop.files = show.desktop
    show_desktop.path = $$CONTENTACTION_TESTDIR/applications
    show_desktop.CONFIG = no_check_exist
    show_desktop.commands = sed \'s%@PATH@%$${CONTENTACTION_TESTDIR}%\' $$PWD/show.desktop.in > $$PWD/show.desktop

    ubermeego_desktop.target = ubermeego.desktop
    ubermeego_desktop.files = ubermeego.desktop
    ubermeego_desktop.path = $$CONTENTACTION_TESTDIR/applications
    ubermeego_desktop.CONFIG = no_check_exist
    ubermeego_desktop.commands = sed \'s%@PATH@%$${CONTENTACTION_TESTDIR}%\' $$PWD/ubermeego.desktop.in > $$PWD/ubermeego.desktop

    upload_desktop.target = upload.desktop
    upload_desktop.files = upload.desktop
    upload_desktop.path = $$CONTENTACTION_TESTDIR/applications
    upload_desktop.CONFIG = no_check_exist
    upload_desktop.commands = sed \'s%@PATH@%$${CONTENTACTION_TESTDIR}%\' $$PWD/upload.desktop.in > $$PWD/upload.desktop

    QMAKE_DISTCLEAN += other.desktop show.desktop ubermeego.desktop upload.desktop

    INSTALLS += other_desktop show_desktop ubermeego_desktop upload_desktop
}

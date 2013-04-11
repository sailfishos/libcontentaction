TEMPLATE = aux

include(../../common.pri)

desktop_tests.path = $$CONTENTACTION_TESTDIR/applications
desktop_tests.files = \
    mimeinfo.cache \
    defaults.list \
    uberexec.desktop \
    unterexec.desktop \
    ubermeego.desktop \
    ubermimeopen.desktop \
    caller.desktop \
    addcontact.desktop \
    contacthandler.desktop \
    emailer.desktop \
    galleryserviceinterface.desktop \
    other.desktop \
    show.desktop \
    upload.desktop \
    fixedparams.desktop \
    plainimageviewer.desktop \
    trackerimageviewer.desktop \
    plainmusicplayer.desktop \
    complexmusic.desktop \
    uriprinter.desktop \
    gallerywithfilename.desktop \
    browser.desktop \
    special-browser.desktop \
    regexpmatcher.desktop \
    ovi-music.desktop
INSTALLS += desktop_tests


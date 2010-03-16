/*!

  \mainpage Libcontentaction

  \brief a library for associating URIs to the set of applicable
  ContentAction::Action objects and triggering the actions.

  \section Overview

  The libcontentaction library retrieves associated actions
  (ContentAction::Action objects) for files, objects stored in Tracker and
  regular expressions in free text.  It may be used to query the default
  action as well.  The Action object can be used to trigger() the action.

  Actions correspond to .desktop files and they can be installed independently
  from libcontentaction.

  In case of file URIs, the library finds out the mime type and uses that as a
  key of the association.  For objects stored in Tracker, the ontology classes
  are used for this purpose.  Finally, the library provides
  ContentAction::Dui::highlightLabel() for adding highlighters to a DuiLabel
  based on the available actions handling regexps.

  \section providing_actions Providing actions

  Actions can target one of the following:

  -# mime types ("image/jpeg")
  -# ontology classes ("x-maemo-tracker/nco-Contact")
  -# regular expressions ("x-maemo-regexp/phonenumber")
     from a pre-defined set

  The library supports launching actions with one of the following methods:

  -# launch a Dui based application using the DuiApplication D-Bus interface
  -# call a D-Bus method specified in the .desktop file
  -# call a Maemo Service Framework method
  -# execute a binary
  -# call the legacy mime_open method

  All D-Bus based invocation methods (except mime_open) require the receiver
  to accept an array of strings.

  To define a new action, drop a .desktop file in /usr/share/applications (or
  in one of the directories in $XDG_DATA_DIRS).

  \subsection writing_desktopfile Writing .desktop files for actions

  Let's take an image viewer for example.

  \verbatim
  [Desktop Entry]
  Type=Application
  Icon=gallery.png
  ;; This is needed to prevent this action appearing in the launcher.
  NotShowIn=X-DUI;
  ;; The action weight, used to sort the actions.  The heavy ones sink to the
  ;; bottom of the list presented to the user.
  X-Maemo-Weight=1000

  ;; Defining a localization method:
  ;; 1. Dui-based
  X-Dui-translation-catalog=gallery_catalog
  X-Dui-logical-id=view_logical_id
  ;; 2. XDG style
  Name=View in gallery
  Name[fi]=Näytä galleriassa

  ;; Defining when this action applies:
  ;; 1. ordinary mimetypes
  MimeType=image/*;text/plain;
  ;; 2. Ontology classes
  MimeType=x-maemo-tracker/nco-Contact;
  ;; 3. pre-defined regexps for the highlighter
  MimeType=x-maemo-regexp/phonenumber;

  ;; Defining how to trigger the action:
  ;; 1. invoke a DuiApplication based program, by calling
  ;; the com.nokia.DuiApplicationIf.launch method
  X-Maemo-Service=org.maemo.gallery_service
  ;; 2. general D-Bus invocation
  X-Maemo-Service=org.maemo.gallery_service
  X-Maemo-Method=org.maemo.galleryinterface.viewimage
  ;; It is possible to specify an optional object path, defaults to '/'.
  X-Maemo-Object-Path=/the/object/path
  ;; Also an action may have optional, fixed string arguments, which get
  ;; prepended to normal arguments.
  X-Maemo-Fixed-Args=foo;bar;baz;
  ;; 3. Maemo Service Framework based action
  ;; NOTE in this case you must not define X-Maemo-Service!
  X-Maemo-Method=com.nokia.imageviewerinterface.showImage
  ;; 4. Plain old exec
  Exec=/usr/bin/gallery %U
  ;; 5. the legacy mime_open method, provided only to wrap old programs into
  ;; actions, don't use it in new applications
  X-Osso-Service=org.maemo.gallery_service
  \endverbatim

  Additionally:

  - All D-Bus methods must accept a list of strings and no other parameters
    (in D-Bus terms, signature="as").  When your function is called, the
    strings will be the URIs (or the free-text snippet) used to construct the
    Action.  (You may have a return value; we'll ignore it.)

  - If you use a the Maemo Service Framework based invocation, then you must
    declare your application to be an implementor of that interface.  For
    this, it's enough to add a "Interface: " line to your D-Bus .service file.
    You might also want to publish your interface in the maemo-services
    package, but it's not needed for libcontentaction.


  \section default_actions Default actions

  If you want your application to be the default application for some mime
  types, contact the libcontentaction implementors.

  \section highlighter Free-text highlighter

  Passing a string to highlight() can be used to discover interesting parts of
  the text.  It returns Match objects identifying the position of the match
  and the possible actions.

  \note

  These actions have different semantics than ordinary Actions.  When
  triggered, they call the method with a single element list containing the
  matched text (as UTF-8).  These are very likely _not_ valid URIs!

*/

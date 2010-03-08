/*!

  \mainpage Libcontentaction

  \brief Libcontentaction is a library for associating uri's to the set of
  applicable ContentAction::Action objects and triggering the actions.

  \section Overview

  Libcontentaction reads the Nepomuk classes of the uri's from Tracker. For
  each class, there is a set of applicable actions. Each action is represented
  by a "<Maemo service framework interface>.<method>" string.

  Libcontentaction can retrive the set of applicable actions and a default
  action for a given set of uris, and return the ContentAction::Action objects
  representing those actions. An ContentAction::Action object can be used for
  triggering the corresponding action.

  The association rules between Nepomuk classes and applicable actions are
  specified via .xml files in `/etc/contentaction/actions.d' (this location
  might be overridden using the $CONTENTACTION_ACTIONS environment variable).
  The format is illustrated by the next example:

  \code
  <?xml version="1.0"?>
  <actions>
    <class name="http://www.tracker-project.org/temp/nmm#MusicPiece">
      <action name="com.example.musicplayer.play"
              weight="100"/>
    </class>
    <class name="http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Image">
      <action name="com.example.pictureviewer.showImage"
              weight="100"/>
      <action name="your.service.print" weight="200"/>
    </class>
  </actions>
  \endcode

  \section provider_nepomuk Providing actions for Tracker URI:s

  To define a new action applicable to Tracker URI:s, the following steps are
  needed:

  - You must define a DUI Service FW interface, and you must declare your
    application to be an implementor of that interface. For this, it's enough
    to add a "Interface: " line to your D-Bus .service file. You might also
    want to publish your interface in the maemo-services package, but it's not
    needed for libcontentaction.

  - The interface must contain a D-Bus function taking in a list of strings
    and no other parameters (in D-Bus terms, "as"). When your function is
    called, the strings will be the Tracker URI:s. It is essential that the
    D-Bus signature of the function is excatly this. (You may have a return
    value; we'll ignore it.)

  - The object with the D-Bus object path / must implement the aforementioned
    function of the aforementioned interface.

  - The mapping of one or many Nepomuk class names to your Service FW
    interface + function name needs to be added to the libcontentaction0
    package (contact the implementors for this). The configuration file format
    is still evolving; in the future it will be possible to install the
    configuration files separately from libcontentaction0.

  \section provider_mime Providing actions for files

  To define a new action applicable to files, the following steps are needed:

  - You need to manifest the Mime types your application handles in the
    .desktop file of your application (the MimeType= line).

  - Provide one of the following three alternatives so what we can launch your
    application:
      - The line X-Maemo-Service=my.dbus.name in your .desktop file, and your
      D-Bus object with path /org/maemo/dui implementing a function
      com.nokia.DuiApplicationIf.launch(as). A patch is pending for DUI to do
      this automatically for you. Read the Dui documentation on how to process
      the parameters. The parameters will be file URI:s.
      - Legacy: The line X-Osso-Service=my.dbus.name in your .desktop file,
      and your D-Bus object with path / implementing a function mime_open. The
      parameters will be file URI:s.
      - The line Exec=my-program %U in your .desktop.file. Libcontentaction
      will execute your command with the file URI's.

  - If you want your application to be the default application for some mime
    types, contact the libcontentaction implementors.

  \section highlighter Free-text highlighter

  Passing a string to highlight() can be used to discover interesting parts of
  the text.  It returns Match objects identifying the position of the match
  and the possible actions.

  \note

  These actions have different semantics than ordinary Actions.  When
  triggered, they call the service method with a single string argument
  containing the matched text (as UTF-8).  These are very likely _not_ valid
  Tracker URIs (subjects)!

  \subsection Configuring highlighting

  Service implementors who wish to provide new `highlightable' parts should
  drop a .xml file in the `/etc/contentaction/actions.d' directory.  The
  schema is similar to the example above, except that instead of <class>
  elements, one should specify <highlight> elements, containing the regexp to
  match for, and the applicable actions as their children.  Example:

  \code
  <?xml version="1.0"?>
  <actions>
    <highlight regexp="[^ @]+@([^ .]+)(\.[^ .]+)*">
      <action name="com.example.mua.composeMailTo"/>
    </highlight>
    <highlight regexp="\+?\d+([- ]\d+)*">
      <action name="com.example.phone.call"/>
      <action name="com.example.addressbook.add"/>
    </highlight>
  </actions>
  \endcode

  \section Localization

  Action::localizedName() might be used to get human readable, localized
  action names.

  Action providers who would like to provide also translations should:

  1. Add a <translation> element to their .xml configuration file in
     `/etc/contentaction/actions.d'.  Example:

     \code
     <translation qm="basename_of_the_qm_file"/>
     \endcode

     Alternatively, the "qm" attribute might contain an absolute file name.
     For testing purposes it is possible to set the search path via the
     CONTENTACTION_L10N_PATH environment variable (containing colon-separated
     directories).

  2. Create the .ts files (translation sources) without any context name.
     Example:

     \code
     <?xml version="1.0" encoding="utf-8"?>
     <!DOCTYPE TS>
     <TS version="2.0" language="en_US">
     <context>
       <message>
         <source>com.example.mua.composeMailTo</source>
         <translation>Send mail to</translation>
       </message>
       <message>
         <source>com.example.phone.call</source>
         <translation>Call on the phone</translation>
       </message>
       <message>
         <source>com.example.addressbook.add</source>
         <translation>Add to my contacts</translation>
       </message>
     </context>
     </TS>
     \endcode

  3. Install the .qm files in `/usr/share/contentaction/l10n' (or elsewhere if
     an absolute filename is used).

  For localization of .desktop file based actions please refer to libdui
  documentation.

*/

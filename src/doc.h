/*!

\mainpage libcontentaction

\brief a library for associating files, Tracker URIs and text snippets to a
set of applicable ContentAction::Action objects and triggering the actions.

\section overview Overview

The libcontentaction library retrieves associated actions
(ContentAction::Action objects) for files, objects stored in Tracker and
regular expressions in free text.  It may be used to query the default
action as well.  The Action object can be used to trigger() the action.

Actions correspond to .desktop files and they can be installed independently
from libcontentaction.

In case of file URIs, the library finds out the mime type and uses that as a
key of the association.  For objects stored in Tracker, custom mime types
targeting them are added.  Finally, the library provides
ContentAction::highlightLabel() for adding highlighters to a MLabel based on
the available actions handling regexps (e.g., phone numbers and e-mail addresses).

Actions can target one of the following:

-# mime types (\c image/jpeg)
-# Tracker-query based conditions (\c x-maemo-nepomuk/contact)
-# regular expressions (\c x-maemo-highlight/phonenumber)
-# URI schemes (\c x-maemo-urischeme/mailto)

\section howitworks How libcontentaction works

libcontentaction = \ref xdgmimehandling "XDG mime type handling"
		 + \ref customlaunch
		 + \ref custommime "Custom mime types"
		 + \ref highlighter

The following sections will describe how libcontentaction extends the XDG mime
type handling system.

\section xdgmimehandling XDG mime type handling: a simplified overview

XDG mime type handling associates files to applications which can open them.
This is done with mime types; each file has an associated mime type, and
applications define which mime types they can handle.

Applications define which mime types they handle in the .desktop file they
install to /usr/share/applications.  (See also \c $XDG_DATA_DIRS and \c
$XDG_DATA_HOME in the XDG Base Directory specification.)

An example .desktop file, /usr/share/applications/myimageviewer.desktop

\verbatim
[Desktop Entry]
Type=Application
Name=MyImageViewer
Name[en]=My Image Viewer
Name[fi]=Kuvannäyttäjäni
Exec=/usr/bin/myimgview --view %U
Icon=myimgview.png
MimeType=image/jpeg;image/png;
\endverbatim

The \c MimeType line contains the list of mime types this application handles.
Wildcards (e.g., \c image/*) are allowed.

The \c Exec line defines how the application is executed.  When the action is
launched, \%U will be substituted by a list of image URIs to open (e.g., file:///home/me/myimage.jpg).

The information about which applications handle each mime type is gathered
into /usr/share/applications/mimeinfo.cache.  This is done by a tool called
update-desktop-database.  When a .desktop file is installed by a debian
package, it is automatically called by a dpkg trigger.

Example contents of mimeinfo.cache:
\verbatim
[MIME Cache]
image/jpeg=myimageviewer.desktop;someotherviewer.desktop;
image/png=myimageviewer.desktop
\endverbatim

Default applications for each mime type are declared in
/usr/share/applications/defaults.list.  Each user might also have an own
defaults.list, overriding the global one.

Example contents of defaults.list:
\verbatim
[Default Applications]
image/jpeg=someotherviewer.desktop
image/png=myimageviewer.desktop
\endverbatim

defaults.list is installed by libcontentaction-data.  If you want your
application to be the default application for some mime types, contact the
libcontentaction implementors.

Libcontentaction is compatible with XDG mime type handling.  If you install
the example file myimageviewer.desktop into /usr/share/applications,
MyImageViewer will appear as an action for jpeg and png images.  For most
cases, this is all you need.  However, you can also utilize our \ref customlaunch.

More information:

<a href="http://en.wikipedia.org/wiki/Internet_media_type">Mime types</a>

<a href="http://standards.freedesktop.org/desktop-entry-spec/latest/">Desktop entry specification</a>

<a href="http://freedesktop.org/wiki/Software/desktop-file-utils">Desktop file utils (incl. update-desktop-database)</a>

<a href="http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html"> XDG Base Directory specification</a>

\section customlaunch Custom launch methods

In addition to the Exec line, .desktop files can specify custom ways to launch
the application.  The launch methods supported by libcontentaction are:

-# launch a MeeGoTouch based application using the MApplication D-Bus interface
  - Define: \c X-Maemo-Service=my.bus.name
  - Optionally: \c X-Maemo-Fixed-Args=my;fixed;args
-# call a D-Bus method specified in the .desktop file
  - Define: \c X-Maemo-Service=my.bus.name
  - Define: \c X-Maemo-Method=com.my.interface.Method
  - Optionally: \c X-Maemo-Object-Path=/the/object/path (default: /)
  - Optionally: \c X-Maemo-Fixed-Args=my;fixed;args
-# call a MeeGo Service Framework method
  - Define: \c X-Maemo-Method=com.my.interface.Method
  - Optionally: \c X-Maemo-Fixed-Args=my;fixed;args
  - Note: Do not define \c X-Maemo-Service
-# execute a binary
  - Define: \c Exec=command-to-execute fixedarg1 \%U fixedarg2
  - No need for \c X-Maemo-Fixed-Args
-# call the legacy mime_open method
  - Define: \c X-Osso-Service=my.bus.name

All D-Bus based invocation methods (except mime_open) require the receiver to
accept an array of strings (in D-Bus terms, signature="as").  When your
function is called, the strings will be the URIs (or the free-text snippet)
used to construct the Action, as well as the fixed parameters you might have
specified.  You may have a return value; we will ignore it.

If you use a the MeeGo Service Framework based invocation, you must
declare your application to be an implementor of the interface in question.  For this,
it is enough to add a "Interface: " line to your D-Bus .service file.  You
might also want to publish your interface in the meego-services package, but
it is not needed for libcontentaction.

If your application is running, D-Bus based launching will deliver the
function call to the running instance.  If your application is not running,
D-Bus can autolaunch it the way you define in your .service file.  This makes
it possible to have only one instance of your application running without any
extra code.

See also \ref exampledesktop.

More information:

<a href="http://www.freedesktop.org/wiki/Software/dbus">D-Bus</a>

<a href="http://dbus.freedesktop.org/doc/dbus-specification.html">Message Bus Starting Services</a>

<a href="http://apidocs.meego.com/mtf/index.html">MeeGoTouch</a>

\section custommime Custom mime types for Tracker URIs

\attention
The following section may be subject to changes!

The XDG mime type handling associates files to applications cabable of opening
them, but not everything is represented a file.  For example, contacts are not
represented as files but as objects in a sematic data store, Tracker.  Moreover,
it is often necessary to define actions to a subset of certain objects.  For
example, the "call" action is only applicable to contacts which have an
associated phone number.

To this end, libcontentaction can find actions for Tracker URIs.  Tracker URI
is the id of an object in the Tracker data store.

If an URI represents a normal file (e.g., an image), the actions found via XDG
mime type handling are added to the list of applicable actions.  This makes
libcontentaction compatible with XDG mime type handling; applications
declaring actions for normal files do not need to know about Tracker URIs.

Libcontentaction defines so called "tracker conditions" for finding out if an
URI is of interest.  The conditions are mapped into custom mime types
(\c "x-maemo-nepomuk/...").

The conditions are defined in .xml files installed in /usr/share/contentaction
(or a location specified by $CONTENTACTION_ACTIONS).  Each condition defines a
SparQL snippet for querying whether a given URI satisfies the condition.

E.g., the following .xml adds new mime types \c x-maemo-nepomuk/image, \c
x-maemo-nepomuk/contact and \c x-maemo-nepomuk/contact-with-phone-number.

\code
<actions>
  <tracker-condition name="image">
    { ?uri a nfo:Image . }
  </tracker-condition>

  <tracker-condition name="contact">
    { ?uri a nco:Contact . }
  </tracker-condition>

  <tracker-condition name="contact-with-phone-number"><![CDATA[
    { ?uri a nco:Contact ;
           nco:hasPhoneNumber ?phone . }
  ]]></tracker-condition>
</actions>
\endcode

A condition is evaluated by executing the following SparQL query:

\code
SELECT 1 {
   SNIPPET
   FILTER(?uri = <the-uri-to-verify-against>)
}
\endcode

The condition applies if the query returns non-zero rows.

An application can now define in its .desktop file that it handles a custom
mime type.  When launched, the application will receive as a parameter the
Tracker URI of the object to be opened.  The application needs to query all
the needed information (e.g., the name and phone number for a contact) from
Tracker.

If your application wants to receive Tracker URIs instead of file URIs as
parameters, a Tracker condition can overlap normal mime types.  For example,
applications handling the \c x-maemo-nepomuk/image mime type will get Tracker
URIs and applications handling \c image/jpeg will get file URIs as parameters.
Both applications will appear as applicable actions for images.

More information:

<a href="http://live.gnome.org/Tracker/Documentation">Tracker</a>

<a href="http://www.semanticdesktop.org/ontologies/">Nepomuk ontologies</a>

<a href="http://www.w3.org/TR/rdf-sparql-query/">SparQL, the query language</a>

\section custommimescheme Custom mime types for URI schemes

Libcontentaction is also able to dispatch URIs based on the scheme
(ContentAction::Action::actionsForScheme).  To this end, applications can
define they handle a custom mime type, e.g., \c x-maemo-urischeme/http.  When
actionsForScheme("http://www.example.com") is called, applications declaring
\c x-maemo-urischeme/http will appear in the list of applicable actions.  When
launched, an action will get the string "http://www.example.com" as a
parameter.

\section highlighter Free-text highlighter

\attention
The following section may be subject to changes!

Passing a MLabel* to ContentAction::Action::highlightLabel() will add a
MLabelHighlighter highlighting interesting elements inside the label.  When
the user clicks a highlighted element, the default action for it is launched.
When the user long-clicks a highlighted element, a pop-up menu containing the
applicable actions is shown.  When the user clicks an item in the menu, the
corresponding action is launched.

These actions have different semantics than ordinary Actions.  When
triggered, they call the method with a single element list containing the
matched text (as UTF-8).  These are very likely not valid URIs!

Similarly to Tracker conditions, regexps are also defined in XML files
residing in \c /usr/share/contentaction (unless overridden with
$CONTENTACTION_ACTIONS).

E.g., the following .xml adds new mime types \c
x-maemo-highlight/email-address and \c x-maemo-nepomuk/phone-number.

\code
<actions>
  <highlight regexp="[^\s@]+@([^\s.]+)(\.[^\s.]+)*" name="email-address"/>
  <highlight regexp="\+?\d+([- ]\d+)*" name="phone-number"/>
</actions>
\endcode

Applications can now define in their .desktop files that they handle these
custom mime types.  When launched, they will get a string matching the regexp
as a parameter. For example, an application handling \c x-maemo-highlight/phone-number
might get "+ 123 456-789" as a parameter.

More information:

<a href="http://apidocs.meego.com/mtf/class_m_label.html">MLabel documentation</a>

\section exampledesktop An example .desktop file

The following example summarizes the interesting .desktop file fields from
libcontentaction point of view:

\verbatim
[Desktop Entry]
Type=Application
Icon=gallery.png
;; This is needed to prevent this action appearing in the launcher.
NotShowIn=X-MeeGo;

;; Defining a localization method:
;; 1. MeeGoTouch-based
X-MeeGo-Translation-Catalog=gallery_catalog
X-MeeGo-Logical-Id=view_logical_id
;; 2. XDG style
Name=View in gallery
Name[fi]=Näytä galleriassa

;; Defining when this action applies:
;; 1. ordinary mimetypes
MimeType=image/*;text/plain;
;; 2. Tracker-query based conditions
MimeType=x-maemo-nepomuk/contact;
;; 3. pre-defined regexps for the highlighter
MimeType=x-maemo-highlight/phonenumber;
;; 4. URI schemes
MimeType=x-maemo-urischeme/mailto;

;; Defining how to trigger the action:
;; 1. invoke a MApplication based program, by calling
;; the com.nokia.MApplicationIf.launch method
X-Maemo-Service=org.maemo.gallery_service
;; 2. general D-Bus invocation; define a bus name and a function
X-Maemo-Service=org.maemo.gallery_service
X-Maemo-Method=org.maemo.galleryinterface.viewimage
;; It is possible to specify an optional object path, defaults to '/'.
X-Maemo-Object-Path=/the/object/path
;; Also an action may have optional, fixed string arguments, which get
;; prepended to normal arguments. (This applies to all launch methods.)
X-Maemo-Fixed-Args=foo;bar;baz;
;; 3. MeeGo Service Framework based action
;; NOTE in this case you must not define X-Maemo-Service!
X-Maemo-Method=com.nokia.imageviewerinterface.showImage
;; 4. Plain old exec
Exec=/usr/bin/gallery %U
;; 5. the legacy mime_open method, provided only to wrap old programs into
;; actions, don't use it in new applications
X-Osso-Service=org.maemo.gallery_service
\endverbatim

More information:

<a href="http://apidocs.meego.com/mtf/i18n.html#translationsystem">MeeGoTouch translation system</a>

\section lcatool lca-tool

lca-tool is a command-line utility which can be used as a development and
testing tool by action implementors.  It is installed by the libcontentaction0
package.  Run lca-tool without parameters to display the help message.

An example workflow for testing whether an image viewer has successfully
declared the \c image/jpeg mime type:

\verbatim
$ cp myimageviewer.desktop /usr/share/applications
$ update-desktop-database /usr/share/applications
$ lca-tool --file --printmimes file:///home/me/myimage.jpg
image/jpeg
$ lca-tool --file --print file:///home/me/myimage.jpg
someotherviewer
myimageviewer
$ lca-tool --file --trigger myimageviewer file:///home/me/myimage.jpg
(MyImageViewer should launch)
\endverbatim

*/

/*!

\mainpage libcontentaction

\brief This library associates files and text snippets to a
set of applicable ContentAction::Action objects and triggers the actions.

\section overview Overview

The libcontentaction library retrieves associated actions (ContentAction::Action
objects) for files and regular expressions (regexp)
in free text.  It may also be used to query the default action.  The Action
object can be used to trigger() the action.

Actions correspond to .desktop files, and they can be installed independently
from libcontentaction.

For file URIs, the library finds out the MIME type and uses it as a key of the
association.

Actions can target one of the following:

-# MIME types (\c image/jpeg)
-# regular expressions (\c x-maemo-highlight/phonenumber)
-# URI schemes (\c x-scheme-handler/mailto)

\section detaileddescription Detailed description

libcontentaction = \ref xdgmimehandling "XDG MIME type handling"
		 + \ref customlaunch
		 + \ref custommime "Custom MIME types"
		 + \ref highlighter

The following sections describe how libcontentaction extends the XDG MIME
type handling system.

\section xdgmimehandling XDG MIME type handling

XDG MIME type handling associates files with applications which can open them.
Each file has an associated MIME type, and applications define which MIME types
they can handle.

Applications define which MIME types they handle in the .desktop file
installed to /usr/share/applications.  (See also \c $XDG_DATA_DIRS and \c
$XDG_DATA_HOME in the XDG Base Directory specification.)

Example of a .desktop file called /usr/share/applications/myimageviewer.desktop:

\verbatim
[Desktop Entry]
Type=Application
Name=MyImageViewer
Name[en]=My Image Viewer
Name[fi]=Kuvankatselu
Exec=/usr/bin/myimgview --view %U
Icon=myimgview.png
MimeType=image/jpeg;image/png;
\endverbatim

The \c MimeType line contains the list of MIME types that the application
handles.  Wildcards (such as \c image/*) are allowed.

The \c Exec line defines how to execute the application.  When the action is
launched, \%U is replaced with a list of image URIs to open (such as
file:///home/me/myimage.jpg).

The update-desktop-database tool gathers information on which application
handles which MIME type into /usr/share/applications/mimeinfo.cache.  When a
.desktop file is installed by a debian package, it is automatically called by a
dpkg trigger.

Example of mimeinfo.cache:
\verbatim
[MIME Cache]
image/jpeg=myimageviewer.desktop;someotherviewer.desktop;
image/png=myimageviewer.desktop
\endverbatim

Default applications for each MIME type are declared in
/usr/share/applications/defaults.list.  Each user might also have their own
defaults.list, overriding the global one.

Example of defaults.list:
\verbatim
[Default Applications]
image/jpeg=someotherviewer.desktop
image/png=myimageviewer.desktop
\endverbatim

The defaults.list is installed by the package libcontentaction-data.

The libcontentaction library is compatible with XDG MIME type handling.  If you
install the example file myimageviewer.desktop into /usr/share/applications,
MyImageViewer appears as an action for jpeg and png images.  For most cases,
this is all you need.  However, libcontentaction also provides you with custom
launch methods. For more information, see \ref customlaunch.

More information:

<a href="http://en.wikipedia.org/wiki/Internet_media_type">MIME types</a>

<a href="http://standards.freedesktop.org/desktop-entry-spec/latest/">Desktop entry specification</a>

<a href="http://freedesktop.org/wiki/Software/desktop-file-utils">Desktop file utils (including update-desktop-database)</a>

<a href="http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html"> XDG Base Directory specification</a>

\section customlaunch Custom launch methods

In addition to the Exec line, .desktop files can specify custom ways to launch
the application.  The launch methods supported by libcontentaction are:

-# launching application using D-Bus interface
  - Define: \c X-Maemo-Service=my.bus.name
  - Optionally: \c X-Maemo-Fixed-Args=my;fixed;args
-# calling a D-Bus method specified in the .desktop file
  - Define: \c X-Maemo-Service=my.bus.name
  - Define: \c X-Maemo-Method=com.my.interface.Method
  - Optionally: \c X-Maemo-Object-Path=/the/object/path (default: /)
  - Optionally: \c X-Maemo-Fixed-Args=my;fixed;args
-# calling a D-Bus method
  - Define: \c X-Maemo-Method=com.my.interface.Method
  - Optionally: \c X-Maemo-Fixed-Args=my;fixed;args
  - Note: Do not define \c X-Maemo-Service
-# executing a binary
  - Define: \c Exec=command-to-execute fixedarg1 \%U fixedarg2
  - No need for \c X-Maemo-Fixed-Args
  - Must have Type=Application
-# calling the legacy mime_open method
  - Define: \c X-Osso-Service=my.bus.name

All D-Bus based invocation methods (except for mime_open) require the receiver
to accept an array of strings (in D-Bus terms, signature="as").  When your
function is called, the strings are the URIs (or the free-text snippet)
used to construct the Action, as well as the fixed parameters you may have
specified.  If there is a return value, it is ignored.

If your application is running, D-Bus based launching delivers the
function call to the running instance.  If your application is not running,
D-Bus can autolaunch it as defined in your .service file.  This makes
it possible to have only one instance of your application running without any
extra code.

For more information, see \ref exampledesktop.

More information:

<a href="http://www.freedesktop.org/wiki/Software/dbus">D-Bus</a>

<a href="http://dbus.freedesktop.org/doc/dbus-specification.html">Message Bus Starting Services</a>


\section custommimescheme Custom MIME types for URI schemes

The libcontentaction library is also able to dispatch URIs based on the scheme
(ContentAction::Action::actionsForScheme).  To this end, applications can
define that they handle a custom MIME type, for example,
\c x-scheme-handler/http.  When actionsForScheme("http://www.example.com") is
called, applications declaring \c x-scheme-handler/http appear in the list of
applicable actions.  When launched, an action gets the string
"http://www.example.com" as a parameter.

\section highlighter Free-text highlighter

FIXME: missing proper documentation for highlighting.

Regexps are defined in .xml files
located in \c /usr/share/contentaction (unless overridden with
$CONTENTACTION_ACTIONS).

For example, the following .xml adds new MIME types \c
x-maemo-highlight/email-address and \c x-maemo-nepomuk/phone-number.

\code
<actions>
  <highlight regexp="[^\s@]+@([^\s.]+)(\.[^\s.]+)*" name="email-address"/>
  <highlight regexp="\+?\d+([- ]\d+)*" name="phone-number"/>
</actions>
\endcode

Applications can now define in their .desktop files that they handle these
custom MIME types.  When launched, they get a string which matches the regular
expression as a parameter. For example, an application handling
\c x-maemo-highlight/phone-number might get "+ 123 456-789" as a parameter.

\section exampledesktop An example .desktop file

The following example illustrates the interesting .desktop file fields from the
libcontentaction point of view:

\verbatim
[Desktop Entry]
Type=Application
Icon=gallery.png
;; This is needed to prevent this action from appearing in the launcher.
NotShowIn=X-MeeGo;

;; Defining a localisation method:
;; 1. MeeGoTouch-based
X-MeeGo-Translation-Catalog=gallery_catalog
X-MeeGo-Logical-Id=view_logical_id
;; 2. XDG style
Name=View in gallery
Name[fi]=Avaa galleriassa

;; Defining when this action applies:
;; 1. ordinary mimetypes
MimeType=image/*;text/plain;
;; 2. pre-defined regexps for the highlighter
MimeType=x-maemo-highlight/phonenumber;
;; 3. URI schemes
MimeType=x-scheme-handler/mailto;

;; Defining how to trigger the action:
;; 1. invoke a MApplication based program by calling
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
;; If you use this launch method, you need to have also Type=Application
Exec=/usr/bin/gallery %U
;; 5. the legacy mime_open method, provided only to wrap old programs into
;; actions, do not use it in new applications
X-Osso-Service=org.maemo.gallery_service
\endverbatim

More information:

<a href="http://apidocs.meego.com/mtf/i18n.html#translationsystem">MeeGoTouch translation system</a>

\section lcatool lca-tool

The lca-tool is a command-line utility which can be used as a development and
testing tool by action implementors.

Use the lca-tool to:
- list the applicable actions for a file or a string with a scheme.
- trigger an applicable action for a file or a string with a scheme.
- launch an application (given its .desktop file) with parameters.
- search for interesting items to highlight in free text.

The lca-tool is installed by the libcontentaction0 package.  To display the help
message, run lca-tool without parameters.

Example workflow for testing that an image viewer has successfully
declared the \c image/jpeg MIME type:

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

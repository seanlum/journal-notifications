# journal-notifications
A program for sending journalctl events to the desktop session.

## Building<br>
**Dependencies:**<br>
`make pkgconf libsystemd libnotify glib2.0 gdk-pixbuf2`<br>
<br>

**Building:**
```
$ make
```

**Installing:** (Installs journal-notifications and an autostart script called "Journal Notifications.desktop")
```
$ make install
```

**Cleaning:**
```
$ make clean
```

**Usage:**
```
$ journal-notify [<field_name> [, <field_name>...]]
$ journal-notify sshd sudo audit
```


**Uninstalling:**
```
$ make uninstall
```

**Files:**
```
Makefile                              # Builder script
res/Journal Notifications.desktop     # Autostart entry. Installed to ~/.config/autostart/
src/journal-notifications.c           # Main program source. Compiled to bin/journal-notifications
bin/journal-notifications             # Main program. Installed to ~/.local/bin/
```

**TODO:**<br>
- Create ability to filter messages from matching identifiers
- Create rule matching functionality utilizing PCRE2
- Create rule file matching expressions with fields
# 2020-03-16

## Bug Fixes

+ Fixed the bug that the system notification cannot be disabled
+ Fixed video device enumeration for higher framerates
+ More reliable high dpi adaption
+ Prevent crash when video device events happen with no account

## New Features

+ Add type indicator
+ Movable splitter in main view
+ Connectivity improvement for calls and messages

# 2019-12-13

## Bug Fixes

+ Fixed the bug that caused lag on call overlay
+ Fixed a typo on about dialog

## New Features

+ Add various useful shortcuts

# 2019-11-20

## Bug Fixes

+ Fixed bug that prevented pasting multiline text into chat

## New Features

+ Calls now automatically un-hold when the conversation is selected
+ SIP dialpad now has A,B,C,D dtmf options

# 2019-11-19

## Bug Fixes

+ Fixed notifications popping up for outgoing calls
+ Removing conference participant selects a remaining participant conversation

# 2019-11-15

## Notes

Change version name to 'Free as in Freedom'
Linux: ffmpeg now embedded in official package for auto bitrate + hardware acceleration

## Features

+ callview: rework conference ui ([#1052](https://git.jami.net/savoirfairelinux/ring-client-gnome/issues/1052))
+ JAMS: support login to an account manager
+ wizard: re-work account creation
+ chatview: add video recorder
+ chatview: add audio recorder
+ conversation: automatically accepts < 20Mb files
+ video: auto adapt bitrate
+ Beta version
+ Change logs
+ Save draft text messages

## Bugfixes

+ Fix multiple crash with SIP accounts
+ Fix DTMF for SIP
+ chatview: show generated avatar into the chatview ([#947](https://git.jami.net/savoirfairelinux/ring-client-gnome/issues/947))
+ Fix account migration
+ Fix alignment in settings view
+ call: handle PEER_BUSY state
+ lrc: fix clearAllHistory
+ avmodel: sort framerates
+ sip: fix temporary item removal
+ avmodel: getCurrentRenderedDevice support conferences
+ upnp: now async
+ some deadlocks are fixed
+ some segfault are fixed
+ sip_transport: avoid PJ_ENOTSUITABLE when ipv4<->ipv6
+ sipcall: fix rotation
+ ffmpeg: handle ebusy when opening devices
+ file transfer: fix cancel
+ conference: fix color inversion during conferences
+ lan: improve pjsip behavior in a LAN
+ [trustrequests: handle removed contacts](https://git.jami.net/savoirfairelinux/ring-daemon/issues/129)
+ dbus: only authorize one daemon per DBUS_SESSION_BUS_ADDRESS
+ Fixed crash when user has no account
+ Fixed display name changes not saving
+ Fixed not being able to change back to camera from screen share
+ Fixed utf-8 handling on display names

## Internal changes

+ chatview code is now in LRC and shared with the desktop clients
+ database: migrate to per account database
+ avmodel: optionally switchInput using a callId
+ video sender: send only 1 keyframe at start
+ contrib: various bump (opendht, upnp, ffmpeg, etc)
+ p2p: use one IceTransport by sub transfer
+ decoder: set fpsprobesize, use default probesize
+ ice: enable aggressive nomination to avoid latencies
+ accel: remove libdrm code
+ sipvoiplink: remove wait for completed transactions in dtor
+ Remove some thread creations
+ replace restbed by restinio
+ namedirectory: don't create temporary items during lookup
+ file transfer: use different ice for each transfer
+ manager: allow switchInput on conference


# 2019-08-24

## Features

+ [Erase data securely before removing account](https://git.jami.net/savoirfairelinux/ring-daemon/issues/60)
+ [Negotiate calls in TCP and UDP and prefer TCP if necessary](https://git.jami.net/savoirfairelinux/ring-daemon/issues/103)
+ Improve negotiation for p2p file transfer
+ Auto change the video quality
+ Add hardware acceleration support for NVidia
+ SIP fix SMS issues

## Bugfixes

+ Improve connectivityChange detection and account switching.
+ Translate strings from daemon.
+ Sort resolutions by width
+ [Sort conversations when clearing history](https://git.jami.net/savoirfairelinux/ring-lrc/issues/411)
+ Fix subscriptions for new contacts
+ Hangup if contact is deleted
+ [Various deadlocks](https://git.jami.net/savoirfairelinux/ring-daemon/issues/120)

## Internal changes

+ LRC remove unused code
+ Change from enableAccount() to setAccountEnabled()
+ Update msgpack, gnutls, opendht
+ Rewrite code for UPnP support

# 2019-06-20

## Bugfixes

+ Improve name registration errors detection
+ Improve SIP text/plain detection
+ Fix temporary item when copy/paste a full ring id
+ SIP: Fix online status
+ [Fix audio recorder](https://git.jami.net/savoirfairelinux/ring-daemon/issues/95)
+ Fix some deadlocks
+ [Fix calls via TURN](https://git.jami.net/savoirfairelinux/ring-daemon/issues/105)
+ [Fix multi devices support for calls](https://git.jami.net/savoirfairelinux/ring-daemon/issues/120)


## Internal changes

+ Cleanup daemon side
+ Update restbed
+ Update opendht to 1.9.5
+ [Improve UPnP implementation](https://git.jami.net/savoirfairelinux/ring-daemon/issues/96)
+ Increase default video bitrate

# 2019-05-16

## Features

+ [Add peer to peer file transfer support](https://git.jami.net/savoirfairelinux/ring-project/issues/486)
+ Advanced settings: add DHT peer discovery support
+ Media Settings: add hardware acceleration support
+ [UPnP add TCP mapping support](https://git.jami.net/savoirfairelinux/ring-daemon/issues/86)

## Bugfixes

+ Name registration: better handling for wrong archive password
+ tls_session: close transport after cleanup
+ sip: check message utf8 validity before emitting signal

## Internal changes

+ Bump OpenDHT to 1.9.4
+ Bump GNUTls to 3.6.7
+ Bump Pjsip to (6b9212dcb4b3f781c1e922ae544b063880bc46ac + patches)
+ Internal renaming from Ring to Jami
+ Use new methods from LRC
+ Fix some data races
+ dring/dbus: unregister signals on exit

# 2019-04-12

## Features

+ (Not linked to the UI for now) Hardware encoding support

## Bugfixes

+ Sets up video streams upon receiving the first video frame.
+ Pulseaudio: start streams when ready

## Internal changes

+ Continue name migration, change data locations, binary names and methods names.
+ Nettle 3.4.1 is now required
+ Support video rotation when recording
+ Some code clean
# libringclient

libringclient is a client library for Jami

For more information about the Jami project, see the following:
- Main website: https://jami.net
- Bug tracker: https://git.jami.net
- Repositories: https://review.jami.net

Installation
==================

libringclient uses Meson for building.

	mkdir build
	cd build
	meson -Dprefix=<> ..
	ninja
	ninja install

Internationalization
====================

To regenerate strings for translations we use lupdate (within root of the project)

lupdate ./src/ -source-language en -ts translations/lrc_en.ts

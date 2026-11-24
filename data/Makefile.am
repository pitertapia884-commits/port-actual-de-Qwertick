desktopdir = $(datadir)/applications
desktop_DATA = $(PACKAGE).desktop

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = $(PACKAGE).png

imagesdir = $(datadir)/$(PACKAGE)
images_DATA = $(PACKAGE).png

soundsdir = $(datadir)/$(PACKAGE)
sounds_DATA = embedded-audio-1-ulaw-44100.wav \
	      embedded-audio-2-pcm-22050.wav

EXTRA_DIST = $(PACKAGE).desktop.in \
	     $(pixmaps_DATA) \
	     $(images_DATA) \
	     $(sounds_DATA)

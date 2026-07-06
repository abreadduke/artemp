.PHONY: all debug install uninstall
all: bin/out
	@echo ""
bin/out: main.c
	mkdir -p bin
	gcc -o bin/out main.c
debug:
	mkdir -p bin
	gcc -o bin/debout -DDEBUG -DDEBUG_MESSAGES main.c
install: bin/out
	mkdir -p $(DESTDIR)/usr/bin $(DESTDIR)/usr/lib/systemd/system $(DESTDIR)/etc/udev/rules.d
	install -m 555 -o root -g root -T bin/out $(DESTDIR)/usr/bin/artempd
	install -m 644 -o root -g root "artempd@.service" $(DESTDIR)/usr/lib/systemd/system
	echo KERNEL==\"ttyUSB*\", TAG+=\"systemd\", ACTION==\"bind\", ENV{SYSTEMD_WANTS}+=\"artempd@%k.service\" > $(DESTDIR)/etc/udev/rules.d/99-artempd.rules
uninstall:
	rm $(DESTDIR)/usr/bin/artempd $(DESTDIR)/usr/lib/systemd/system/artempd@.service $(DESTDIR)/etc/udev/rules.d/99-artempd.rules


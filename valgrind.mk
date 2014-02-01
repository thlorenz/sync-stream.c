# valgrind executed on remote linux machine

VFLAGS = --track-origins=yes --tool=memcheck --leak-check=yes

rsync:
	rsync -ra -e ssh --exclude '/.git' --exclude '/bin' . udesktop:tmp/sync-stream

grind: bin/test/stream
	valgrind $(VFLAGS) $^

grind-report: bin/test/stream
	G_SLICE=always-malloc G_DEBUG=gc-friendly \
	valgrind $(VFLAGS) -v --num-callers=40 --log-file=valgrind.log $^

grind-chunk: bin/test/grind/chunk-new-free
	valgrind $(VFLAGS) $^

rgrind-chunk: rsync
	ssh udesktop 'cd tmp/sync-stream && make clean && make grind-chunk'
	
grind-stream: bin/test/test-stream
	valgrind $(VFLAGS) $^

rgrind-stream: rsync
	ssh udesktop 'cd tmp/sync-stream && make clean && make grind-stream'

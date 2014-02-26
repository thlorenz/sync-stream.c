# valgrind executed on remote linux machine

VFLAGS = --track-origins=yes --tool=memcheck --leak-check=yes --error-exitcode=1

grind: $(TESTS) 
	set -e; for file in $^; do echo "\n\033[00;32m+++ $$file +++\033[00m\n" && valgrind $(VFLAGS) ./$$file; done

rgrind-report: rsync
	ssh udesktop 'cd tmp/sync-stream && make clean && make grind-report'
	scp udesktop:tmp/sync-stream/valgrind.log .
	cat valgrind.log

grind-pipe-thru: bin/pipe-thru-transforms
	valgrind $(VFLAGS) $^

grind-file-stream: bin/file-stream-transform
	valgrind $(VFLAGS) $^

rgrind-file-stream: rsync
	ssh udesktop 'cd tmp/sync-stream && make clean && make grind-file-stream'

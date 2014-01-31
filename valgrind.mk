# valgrind executed on remote linux machine
scp:
	scp -r examples include src test Makefile valgrind.mk udesktop:tmp/sync-stream > /dev/null

grind: bin/test/stream
	valgrind --tool=memcheck --leak-check=yes $^

grind-report: bin/test/stream
	G_SLICE=always-malloc G_DEBUG=gc-friendly \
	valgrind -v --tool=memcheck --leak-check=full --num-callers=40 --log-file=valgrind.log $^

grind-chunk: bin/test/grind/chunk-new-free
	valgrind --tool=memcheck --leak-check=yes $^

rgrind-chunk: scp
	ssh udesktop 'cd tmp/sync-stream && make grind-chunk'
	
grind-stream: bin/test/grind/stream-new-free
	valgrind --tool=memcheck --leak-check=yes $^

rgrind-stream: scp
	ssh udesktop 'cd tmp/sync-stream && make grind-stream'

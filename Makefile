default:
	mkdir -p build
	cd build && cmake .. && make --no-print-directory
	./build/ftp

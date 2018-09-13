all: tunguska 3cc memory_image_3 memory_image_a

tunguska:
	make -C tunguska_sources
3cc:
	make -C tunguska_3cc
memory_image_3:
	make -C memory_image_3cc
memory_image_a:
	make -C tunguska_memory_image_sources

clean:
	make -C tunguska_sources clean
	make -C tunguska_3cc clean
	make -C memory_image_3cc clean
	make -C tunguska_memory_image_sources clean

recycle_fs: real/recycle_fs_origin.c
	gcc real/recycle_fs_origin.c `pkg-config fuse3 --cflags` `pkg-config fuse3 --libs` -o recycle_fs
	

unmount:
	@echo "Unmounting mnt..."
	@sudo fusermount3 -u mnt 2>/dev/null || echo "Sudah tidak ter-mount"

chown:
	@echo "Mengatur ownership folder mnt..."
	@sudo chown -R $(USER):$(USER) mnt || { echo "Gagal chown"; exit 1; }

run: unmount chown recycle_fs
	@echo "Menjalankan recycle_fs di mnt..."
	@if ./recycle_fs mnt; then \
		echo "Mount sukses!"; \
	else \
		echo "Gagal menjalankan FUSE!"; \
		exit 1; \
	fi

clean:
	@echo "Membersihkan binary..."
	@rm -f recycle_fs

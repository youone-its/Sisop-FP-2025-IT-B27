# Final Project Sistem Operasi IT

## Peraturan
1. Waktu pengerjaan dimulai hari Kamis (19 Juni 2025) setelah soal dibagikan hingga hari Rabu (25 Juni 2025) pukul 23.59 WIB.
2. Praktikan diharapkan membuat laporan penjelasan dan penyelesaian soal dalam bentuk Readme(github).
3. Format nama repository github “Sisop-FP-2025-IT-[Kelas][Kelompok]” (contoh:Sisop-FP-2025-IT-A01).
4. Setelah pengerjaan selesai, seluruh source code dan semua script bash, awk, dan file yang berisi cron job ditaruh di github masing - masing kelompok, dan link github dikumpulkan pada form yang disediakan. Pastikan github di setting ke publik.
5. Commit terakhir maksimal 10 menit setelah waktu pengerjaan berakhir. Jika melewati maka akan dinilai berdasarkan commit terakhir.
6. Jika tidak ada pengumuman perubahan soal oleh asisten, maka soal dianggap dapat diselesaikan.
7. Jika ditemukan soal yang tidak dapat diselesaikan, harap menuliskannya pada Readme beserta permasalahan yang ditemukan.
8. Praktikan tidak diperbolehkan menanyakan jawaban dari soal yang diberikan kepada asisten maupun praktikan dari kelompok lainnya.
9. Jika ditemukan indikasi kecurangan dalam bentuk apapun di pengerjaan soal final project, maka nilai dianggap 0.
10. Pengerjaan soal final project sesuai dengan modul yang telah diajarkan.

## Kelompok x

Nama | NRP
--- | ---
Yuan Banny Albyan| 5027241027
Nisrina Bilqis | 5027241054
I Gede Bagus Saka Sinatrya| 5027241088

## Deskripsi Soal

**Tempat Sampah Gebang** adalah sistem file berbasis FUSE yang mengimplementasikan fitur "recycle bin" seperti pada OS modern.  
Saat pengguna menghapus (unlink) file, file **tidak langsung dihapus permanen**, melainkan dipindahkan ke folder tersembunyi `.trash` yang terletak di direktori `$HOME`.

---

### Catatan

Struktur repository:
```
.
├── makefile
├── mnt
├── real                        
│   └── recycle_fs_origin.c     
└── recycle_fs 
```

## Pengerjaan
### 1. Implementasi Tempat Sampah

**Teori**
FUSE (Filesystem in Userspace) memungkinkan user-space program untuk menyediakan sistem berkas virtual. Kerangka kerja Direct-FUSE mendukung beberapa FUSE dan sistem berkas tingkat pengguna kustom di ruang pengguna tanpa memerlukan modul kernel, meningkatkan kinerja rata-rata sebesar 11,9% tanpa overhead yang signifikan (Moody 2018). Saat fungsi `unlink()` dipanggil, secara default file akan dihapus dari sistem. Namun pada proyek ini, operasi unlink di-override agar file hanya dipindahkan ke `.trash`.

Konsep yang digunakan:

- Intercept System Call: Ekstensi berfungsi sepenuhnya di tingkat pengguna dan diinstal tanpa bantuan administrator sistem (Ibel 1998). Adanya fungsi `unlink` di-override agar tidak menghapus file secara permanen. 

- Redirection with Timestamp: File dipindah ke folder `.trash` dengan penambahan timestamp agar tidak bentrok nama.

- Environment Variable `$HOME`: Folder `.trash` disimpan sesuai direktori pengguna yang sedang aktif.


**Solusi**

Modifikasi dilakukan pada fungsi:

```c
static int command_unlink(const char *path)
```
Langkah-langkahnya:

- Mengambil path file yang akan dihapus.
- Membuat folder `~/.trash` jika belum ada.
- Mengambil nama file asli, lalu menambahkan timestamp (YYYYMMDDHHMMSS).
- File dipindahkan `(rename())` ke folder `.trash`.

Contoh hasil:

```bash
/real/data.txt   →   ~/.trash/data.txt_20250625024030
```
Jika terjadi error saat memindahkan, error tersebut akan ditangani dan dilaporkan.

### 2. Operasi Dasar File System

**Teori**

Di FUSE (Filesystem in Userspace) setiap sistem operasi berkas ditentukan oleh serangkaian callback yang “menggantikan” panggilan sistem (sys-call) POSIX. Konsepnya: ketika aplikasi di ruang user menjalankan open(), read(), unlink(), dan sebagainya pada direktori mount-point. Basis data automount berisi informasi tentang titik mount sistem berkas yang akan dipetakan oleh automounter (Bannister 2015). Kemudian kernel FUSE akan meneruskan permintaan itu ke proses Anda melalui fungsi-fungsi di bawah ini

Beberapa fungsi penting yang harus diimplementasikan dalam filesystem berbasis FUSE:

- `getattr` → Mendapatkan metadata file (ukuran, hak akses, dsb).
- `readdir` → Membaca isi direktori.
- `read`, `write`, `open`, `create` → Operasi file dasar.
- `unlink` → Penghapusan (dimodifikasi jadi pindah ke `.trash`).

**Solusi**

Seluruh operasi dasar diimplementasikan di file `recycle_fs_origin.c`.
Fungsi `fullpath()` digunakan sebagai helper untuk menyatukan `real_root` dengan `path` relatif dari FUSE.


**Video Menjalankan Program**

https://github.com/user-attachments/assets/7258e1eb-8634-483d-a73d-f48c2f486f5f

## Daftar Pustaka

- Ibel, M., Alexandrov, A., Scheiman, C., & Schauser, K. (1998). UFO. ACM Transactions on Computer Systems (TOCS), 16, 207 - 233. https://doi.org/10.1145/290409.290410.
- Moody, A., Yu, W., Zhu, Y., Wang, T., Mohror, K., Sato, K., & Khan, M. (2018). Direct-FUSE: Removing the Middleman for High-Performance FUSE File System Support. Proceedings of the 8th International Workshop on Runtime and Operating Systems for Supercomputers. https://doi.org/10.1145/3217189.321719
- Bannister, M. (2015). Directory-Based Information Services: Automounter. 

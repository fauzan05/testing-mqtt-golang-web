# Refactoring: Struktur Folder Asset Static

## Tanggal: 19 November 2025

## Tujuan Refactoring
Mengorganisir file CSS dan JavaScript ke dalam struktur folder yang lebih teratur berdasarkan menu/halaman untuk meningkatkan maintainability dan scalability project.

## Perubahan yang Dilakukan

### 1. Struktur Folder Baru

#### CSS Files
**Sebelum:**
```
web/static/css/
├── styles.css
├── sidebar.css
├── form.css
└── data-conductive.css
```

**Sesudah:**
```
web/static/css/
├── common/              # CSS yang digunakan di banyak halaman
│   ├── styles.css
│   ├── sidebar.css
│   └── form.css
└── pages/               # CSS spesifik per halaman
    ├── dashboard/
    ├── data-conductive/
    │   └── data-conductive.css
    ├── history/
    ├── user/
    └── auth/
```

#### JavaScript Files
**Sebelum:**
```
web/static/js/
├── sidebar.js
├── auth-check.js
├── dashboard.js
├── data-conductive.js
└── login.js
```

**Sesudah:**
```
web/static/js/
├── common/              # JavaScript yang digunakan di banyak halaman
│   ├── sidebar.js
│   └── auth-check.js
└── pages/               # JavaScript spesifik per halaman
    ├── dashboard/
    │   └── dashboard.js
    ├── data-conductive/
    │   └── data-conductive.js
    ├── history/
    │   └── history.js
    ├── user/
    │   └── user.js
    └── auth/
        └── login.js
```

### 2. Perubahan di Router (`internal/delivery/http/router.go`)

#### Perubahan Route URL
**Sebelum:**
```go
// Direct file serving
r.app.Get("/styles.css", ...)
r.app.Get("/sidebar.js", ...)
```

**Sesudah:**
```go
// Organized by type with proper folder structure
r.app.Get("/css/styles.css", ...)
r.app.Get("/js/sidebar.js", ...)
```

#### Konfigurasi Route yang Diupdate
- Common CSS files: `map[string]string` dengan path ke `common/` folder
- Page-specific CSS files: `map[string]string` dengan path ke `pages/[menu]/` folder
- Common JS files: `map[string]string` dengan path ke `common/` folder
- Page-specific JS files: `map[string]string` dengan path ke `pages/[menu]/` folder

### 3. Perubahan di Template Files

#### File yang Diupdate:
1. **`web/templates/header.html`**
   - `/auth-check.js` → `/js/auth-check.js`
   - `/styles.css` → `/css/styles.css`
   - `/sidebar.css` → `/css/sidebar.css`

2. **`web/templates/footer.html`**
   - `/sidebar.js` → `/js/sidebar.js`

3. **`web/templates/dashboard.html`**
   - `/auth-check.js` → `/js/auth-check.js`
   - `/styles.css` → `/css/styles.css`
   - `/sidebar.js` → `/js/sidebar.js`
   - `/dashboard.js` → `/js/dashboard.js`

4. **`web/templates/data-conductive.html`**
   - `/auth-check.js` → `/js/auth-check.js`
   - `/sidebar.css` → `/css/sidebar.css`
   - `/data-conductive.css` → `/css/data-conductive.css`
   - `/sidebar.js` → `/js/sidebar.js`
   - `/data-conductive.js` → `/js/data-conductive.js`

5. **`web/templates/history.html`**
   - `auth-check.js` → `/js/auth-check.js`
   - `styles.css` → `/css/styles.css`
   - `sidebar.css` → `/css/sidebar.css`
   - `sidebar.js` → `/js/sidebar.js`
   - Tambah: `/js/history.js`

6. **`web/templates/user.html`**
   - `auth-check.js` → `/js/auth-check.js`
   - `sidebar.css` → `/css/sidebar.css`
   - `styles.css` → `/css/styles.css`
   - `sidebar.js` → `/js/sidebar.js`
   - Tambah: `/js/user.js`

7. **`web/templates/login.html`**
   - `/login.js` → `/js/login.js`

### 4. File Baru yang Dibuat
- `web/static/js/pages/history/history.js` - JavaScript untuk halaman history
- `docs/FOLDER_STRUCTURE.md` - Dokumentasi struktur folder

## Keuntungan Refactoring

### 1. **Modularitas**
- Setiap halaman memiliki folder sendiri untuk assets
- Mudah untuk mengidentifikasi file mana yang digunakan oleh halaman tertentu

### 2. **Maintainability**
- Lebih mudah menemukan dan mengubah kode
- Clear separation of concerns
- Struktur yang konsisten

### 3. **Scalability**
- Mudah menambah halaman baru
- Pattern yang jelas untuk diikuti
- Tidak ada file yang tercampur di satu folder

### 4. **Organization**
- Common files terpisah dari page-specific files
- Folder hierarchy yang clear
- Konsisten dengan best practices

### 5. **Performance**
- Hanya load assets yang diperlukan per halaman
- Clear caching strategy untuk setiap jenis asset

## URL Routing Pattern

### Pattern Baru:
- Common CSS: `/css/[filename].css`
- Page CSS: `/css/[filename].css`
- Common JS: `/js/[filename].js`
- Page JS: `/js/[filename].js`
- Images: `/images/[filename]`

## Cara Menambah Halaman Baru

1. Buat folder di `web/static/css/pages/[nama-halaman]/`
2. Buat folder di `web/static/js/pages/[nama-halaman]/`
3. Buat file CSS dan JS di folder tersebut
4. Daftarkan route di `router.go` dalam `pageCSSFiles` dan `pageJSFiles`
5. Gunakan di template dengan path `/css/[nama-halaman].css` dan `/js/[nama-halaman].js`

## Breaking Changes
❌ **BREAKING CHANGE**: URL assets berubah dari root path ke subfolder path

### Migration Path:
Semua template HTML sudah diupdate untuk menggunakan path baru. Jika ada custom code yang mereferensi assets, update path-nya:

**Sebelum:**
```html
<link href="/styles.css" rel="stylesheet">
<script src="/sidebar.js"></script>
```

**Sesudah:**
```html
<link href="/css/styles.css" rel="stylesheet">
<script src="/js/sidebar.js"></script>
```

## Testing

✅ Server berhasil dijalankan tanpa error
✅ Semua routes terdaftar dengan benar
✅ File structure sudah sesuai dengan dokumentasi

## Notes untuk Developer

- Semua static files menggunakan `no-cache` header untuk development
- Images menggunakan cache 1 tahun (`max-age=31536000`)
- Gunakan CDN untuk library eksternal
- Untuk production, pertimbangkan menggunakan versioning/hash pada filename

## Dokumentasi Terkait
- `docs/FOLDER_STRUCTURE.md` - Detail struktur folder dan konvensi
- `docs/REFACTORING_SUMMARY.md` - Summary dari refactoring sebelumnya

## Commit Message Suggestion
```
refactor: organize static assets into modular folder structure

- Separate CSS files into common/ and pages/ directories
- Separate JS files into common/ and pages/ directories
- Update router.go to use new folder structure
- Update all HTML templates with new asset paths
- Add documentation for folder structure
- Create history.js placeholder file

BREAKING CHANGE: Asset URLs changed from root to /css/ and /js/ paths
```

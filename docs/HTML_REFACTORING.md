# Refactoring HTML Structure

## Overview
Kode HTML telah direfactor untuk memisahkan CSS dan JavaScript ke dalam file-file terpisah, membuat struktur lebih modular dan mudah dimaintain.

## Struktur Baru

### 1. File Template Reusable

#### Header Templates
- **`header.html`** - Header untuk halaman dengan sidebar (dashboard, data-conductive, history, user)
- **`auth-header.html`** - Header untuk halaman autentikasi (login, register)

#### Footer Templates
- **`footer.html`** - Footer untuk halaman dengan sidebar
- **`auth-footer.html`** - Footer untuk halaman autentikasi

### 2. File CSS Terpisah

```
web/static/css/
├── styles.css              # Global styles untuk semua halaman
├── sidebar.css             # Styles untuk sidebar component
├── form.css                # Styles untuk form elements
└── data-conductive.css     # Styles spesifik untuk data-conductive page
```

### 3. File JavaScript Terpisah

```
web/static/js/
├── auth-check.js           # Script untuk autentikasi check
├── sidebar.js              # Script untuk sidebar component
├── dashboard.js            # Script untuk dashboard page (Chart.js logic)
├── data-conductive.js      # Script untuk data-conductive page
└── login.js                # Script untuk login page
```

## Struktur HTML Page

### Halaman dengan Sidebar (Dashboard, Data Conductive, History, User)

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Page Title - CORE</title>
    
    <!-- Auth Check -->
    <script src="/auth-check.js"></script>
    
    <!-- External CSS -->
    <script src="https://cdn.tailwindcss.com"></script>
    
    <!-- Internal CSS -->
    <link href="/styles.css" rel="stylesheet">
    <link href="/sidebar.css" rel="stylesheet">
    <link href="/page-specific.css" rel="stylesheet">
</head>
<body>
    <div class="app">
        <div id="sidebar-root"></div>
        
        <main class="main" style="margin-left:220px;">
            <!-- Page Content -->
        </main>
    </div>
    
    <!-- Scripts -->
    <script src="/sidebar.js"></script>
    <script src="/page-specific.js"></script>
</body>
</html>
```

### Halaman Autentikasi (Login, Register)

```html
<!DOCTYPE html>
<html lang="id">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Page Title - CORE</title>
    
    <!-- External Libraries -->
    <script src="https://cdn.tailwindcss.com"></script>
    <script src="https://code.jquery.com/jquery-3.7.1.min.js"></script>
    
    <!-- Inline Styles (minimal) -->
    <style>
        /* Page specific styles */
    </style>
</head>
<body class="min-h-screen flex items-center justify-center">
    <!-- Page Content -->
    
    <!-- Scripts -->
    <script src="/page-specific.js"></script>
</body>
</html>
```

## Manfaat Refactoring

### 1. **Separation of Concerns**
- CSS, HTML, dan JavaScript dipisahkan ke file masing-masing
- Lebih mudah untuk menemukan dan mengedit kode

### 2. **Reusability**
- CSS dan JavaScript dapat digunakan ulang di berbagai halaman
- Mengurangi duplikasi kode

### 3. **Maintainability**
- Perubahan pada satu file CSS/JS akan mempengaruhi semua halaman yang menggunakannya
- Lebih mudah untuk debugging

### 4. **Performance**
- Browser dapat cache file CSS dan JS
- Loading lebih cepat untuk halaman berikutnya

### 5. **Code Organization**
- Struktur lebih rapi dan terorganisir
- Mudah untuk scaling project

## Router Configuration

File `router.go` telah diupdate untuk melayani file-file baru:

```go
// CSS Files
cssFiles := []string{"styles.css", "sidebar.css", "form.css", "data-conductive.css"}

// JS Files
jsFiles := []string{"sidebar.js", "auth-check.js", "dashboard.js", "data-conductive.js", "login.js"}

// HTML Templates
htmlFiles := []string{"sidebar.html", "header.html", "footer.html", "auth-header.html", "auth-footer.html"}
```

## File yang Telah Direfactor

1. ✅ `login.html` - JavaScript dipindahkan ke `login.js`
2. ✅ `dashboard.html` - JavaScript dipindahkan ke `dashboard.js`
3. ✅ `data-conductive.html` - CSS dan JavaScript dipindahkan ke file terpisah

## File yang Perlu Direfactor (Opsional)

- `history.html`
- `user.html`
- `register.html`

## Cara Menggunakan

### Menambah Halaman Baru

1. Buat file HTML dengan struktur yang konsisten
2. Buat file CSS spesifik jika diperlukan (`page-name.css`)
3. Buat file JS spesifik jika diperlukan (`page-name.js`)
4. Update `router.go` untuk menambahkan route file CSS dan JS baru

### Mengedit Halaman Existing

1. **Untuk perubahan HTML**: Edit file `.html` di `web/templates/`
2. **Untuk perubahan CSS**: Edit file `.css` di `web/static/css/`
3. **Untuk perubahan JavaScript**: Edit file `.js` di `web/static/js/`

## Best Practices

1. **Gunakan file CSS terpisah** untuk styles yang spesifik untuk satu halaman
2. **Gunakan file JS terpisah** untuk logic yang kompleks
3. **Minimalkan inline styles** - gunakan CSS classes
4. **Gunakan IIFE** untuk JavaScript modules: `(function() { ... })()`
5. **Konsisten dengan naming convention**: `page-name.css`, `page-name.js`

## Cache Control

Semua file static dilayani dengan no-cache headers untuk development:

```go
c.Set("Cache-Control", "no-cache, no-store, must-revalidate")
c.Set("Pragma", "no-cache")
c.Set("Expires", "0")
```

Untuk production, pertimbangkan untuk menggunakan cache dengan versioning.

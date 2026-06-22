# 🌱 Sistem Monitoring Tanah

Website dashboard untuk memantau kondisi tanah secara real-time dengan visualisasi data sensor.

## 📋 Fitur

- **🌐 ESP32 Integration**: Integrasi lengkap dengan ESP32 microcontroller untuk pembacaan sensor real-time
- **📡 Backend API Server**: Node.js server untuk menerima dan menyimpan data dari ESP32
- **📊 Monitoring Real-time**: Menampilkan data suhu, kelembapan, pH tanah, dan fosfor
- **📈 Visualisasi Grafik**: Grafik tren untuk setiap parameter menggunakan Chart.js
- **⚠️ Status Indikator**: Sistem peringatan otomatis (Normal/Perhatian/Bahaya)
- **📝 Tabel Data**: Histori data lengkap dengan timestamp
- **💾 Export Data**: Export data ke format CSV
- **📱 Responsive Design**: Tampilan optimal di desktop, tablet, dan mobile
- **🔄 Auto/Manual Mode**: Bisa menggunakan data real dari ESP32 atau simulasi

## 🚀 Cara Menggunakan

### 1. Membuka Dashboard
Cukup buka file `index.html` di browser Anda dengan:
- Double-click file `index.html`, atau
- Klik kanan → Open with → Browser pilihan Anda

### 2. Melihat Data
Dashboard akan otomatis:
- Menampilkan data sensor di kartu (cards)
- Memperbarui grafik setiap 5 detik
- Menambahkan data ke tabel historis
- Menampilkan status kondisi tanah

### 3. Export Data
- Klik tombol **"Export CSV"** untuk mengunduh data dalam format CSV
- File akan tersimpan dengan nama: `soil-monitoring-[timestamp].csv`

### 4. Menghapus Data
- Klik tombol **"Hapus Data"** untuk membersihkan semua data
- Akan muncul konfirmasi sebelum data dihapus

## ⚙️ Konfigurasi

### Mengubah Threshold Status
Edit file `script.js` bagian `THRESHOLDS`:

```javascript
const THRESHOLDS = {
    temperature: { min: 20, max: 30, ideal: 25 },
    humidity: { min: 40, max: 70, ideal: 60 },
    ph: { min: 6.0, max: 7.5, ideal: 6.5 },
    phosphorus: { min: 30, max: 60, ideal: 45 }
};
```

### Mengubah Interval Update
Edit file `script.js` di bagian init():

```javascript
// Auto-update setiap 5 detik (5000ms)
setInterval(updateSensorReadings, 5000);
```

Ganti `5000` dengan interval yang diinginkan (dalam milidetik).

### Mengubah Jumlah Data Points di Grafik
Edit file `script.js`:

```javascript
const MAX_DATA_POINTS = 20; // Ubah angka ini
```

## 🔌 Integrasi dengan ESP32 dan Sensor Real

Sistem ini **sudah terintegrasi penuh dengan ESP32**! Anda bisa menggunakan data real dari sensor fisik.

### 🚀 Quick Start

#### 1. Setup Hardware ESP32
Lihat panduan lengkap: **[ESP32_SETUP.md](ESP32_SETUP.md)**

**Hardware yang dibutuhkan:**
- ESP32 Development Board
- DHT22 (Temperature & Humidity Sensor)
- Soil Moisture Sensor (Analog)
- pH Sensor Module (Analog)
- NPK/Phosphorus Sensor (Analog)

**Wiring:**
- DHT22 Data → GPIO 4
- Soil Moisture → GPIO 34
- pH Sensor → GPIO 35
- Phosphorus → GPIO 32

#### 2. Setup Backend Server
```bash
cd backend
npm install
npm start
```

Backend akan berjalan di `http://localhost:3000`

Panduan lengkap: **[backend/README.md](backend/README.md)**

#### 3. Upload Firmware ke ESP32
1. Install Arduino IDE dan ESP32 board support
2. Install library: DHT sensor library, ArduinoJson
3. Edit `esp32_soil_monitor/esp32_soil_monitor.ino`:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   const char* serverUrl = "http://192.168.1.100:3000/api/sensor-data";
   ```
4. Upload ke ESP32

#### 4. Buka Dashboard
1. Buka `index.html` di browser
2. Dashboard akan otomatis menerima data dari ESP32
3. Status koneksi ditampilkan di header (● Online / ● Offline)

### 📡 Arsitektur Sistem

```
┌─────────────┐         ┌──────────────┐         ┌─────────────┐
│   ESP32     │  HTTP   │   Backend    │  HTTP   │  Dashboard  │
│   + Sensor  │ ------> │   Server     │ ------> │   Web App   │
│             │  POST   │  (Node.js)   │  GET    │   (HTML/JS) │
└─────────────┘         └──────────────┘         └─────────────┘
   (WiFi)                 Port 3000                 Browser
```

### 🔄 Mode Operasi

**Mode Real-time (Default):**
Frontend mengambil data dari backend API yang menerima data dari ESP32.

**Mode Simulasi:**
Edit `script.js`:
```javascript
const API_CONFIG = {
    baseUrl: 'http://localhost:3000/api',
    useSimulation: true  // Set true untuk simulasi
};
```

### 📚 Dokumentasi Lengkap

- **[ESP32_SETUP.md](ESP32_SETUP.md)** - Setup hardware, wiring, kalibrasi sensor
- **[backend/README.md](backend/README.md)** - API endpoints, konfigurasi server
- **[esp32_soil_monitor.ino](esp32_soil_monitor/esp32_soil_monitor.ino)** - Firmware ESP32

## 📁 Struktur File

```
soil-monitoring/
├── index.html                      # File HTML utama (Dashboard)
├── style.css                       # Styling dan desain
├── script.js                       # Logika frontend (dengan API integration)
├── README.md                       # Dokumentasi utama
├── ESP32_SETUP.md                  # Panduan setup ESP32
│
├── esp32_soil_monitor/
│   └── esp32_soil_monitor.ino     # Firmware ESP32 (Arduino)
│
└── backend/
    ├── server.js                   # Backend API server (Node.js)
    ├── package.json                # Dependencies
    ├── .env                        # Konfigurasi environment
    └── README.md                   # Dokumentasi API
```

## 🎨 Kustomisasi Tampilan

### Mengubah Warna Tema
Edit file `style.css`:

```css
/* Background gradient */
body {
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
}

/* Header gradient */
header {
    background: linear-gradient(135deg, #2ecc71 0%, #27ae60 100%);
}
```

### Mengubah Ikon
Edit file `index.html` di bagian `.card-icon`:

```html
<div class="card-icon">🌡️</div> <!-- Ganti emoji di sini -->
```

## 🛠️ Teknologi yang Digunakan

**Frontend:**
- **HTML5**: Struktur halaman
- **CSS3**: Styling dan animasi
- **JavaScript (Vanilla)**: Logika aplikasi
- **Chart.js**: Library untuk grafik

**Backend:**
- **Node.js**: Runtime JavaScript
- **Express.js**: Web framework
- **CORS**: Cross-origin resource sharing

**Hardware:**
- **ESP32**: Microcontroller (WiFi enabled)
- **DHT22**: Temperature & Humidity sensor
- **Analog Sensors**: Soil moisture, pH, Phosphorus

**Communication:**
- **HTTP/REST API**: ESP32 → Backend → Frontend
- **JSON**: Data format

## 📊 Parameter yang Dimonitor

1. **Suhu Tanah** (°C)
   - Normal: 20-30°C
   - Ideal: 25°C

2. **Kelembapan** (%)
   - Normal: 40-70%
   - Ideal: 60%

3. **pH Tanah**
   - Normal: 6.0-7.5
   - Ideal: 6.5

4. **Fosfor/P** (ppm)
   - Normal: 30-60 ppm
   - Ideal: 45 ppm

## 💡 Tips

- Sesuaikan threshold berdasarkan jenis tanaman yang Anda monitor
- Gunakan server web lokal untuk testing (XAMPP, Live Server, dll.)
- Untuk deployment online, upload ke hosting atau gunakan GitHub Pages
- Backup data CSV secara berkala

## 🤝 Kontribusi & Pengembangan

Fitur yang sudah diimplementasi:
- [x] Integrasi ESP32 dengan sensor real
- [x] Backend API server
- [x] Real-time data monitoring
- [x] Connection status indicator

Fitur yang bisa ditambahkan:
- [ ] Notifikasi alert (email/SMS/Telegram) saat kondisi bahaya
- [ ] Database persisten (SQLite/MongoDB) untuk menyimpan data jangka panjang
- [ ] Login/authentication untuk multi-user
- [ ] Dashboard admin untuk konfigurasi threshold
- [ ] Mobile app (Progressive Web App)
- [ ] Integrasi dengan Google Sheets
- [ ] Prediksi kondisi tanah dengan Machine Learning
- [ ] Multi-device support (beberapa ESP32)
- [ ] Historical data analytics
- [ ] Automatic irrigation control

## 📞 Troubleshooting

### Dashboard tidak menampilkan data
1. Periksa console browser (F12) untuk error messages
2. Pastikan backend server berjalan di `http://localhost:3000`
3. Cek status koneksi di dashboard header (● Online / ● Offline)
4. Verifikasi `API_CONFIG.baseUrl` di `script.js` sesuai dengan server

### ESP32 tidak bisa connect ke WiFi
1. Periksa SSID dan password benar
2. Pastikan WiFi 2.4GHz (ESP32 tidak support 5GHz)
3. Cek Serial Monitor (115200 baud) untuk error messages
4. Pastikan router tidak memblokir device baru

### Backend error "EADDRINUSE"
Port 3000 sudah digunakan aplikasi lain. Edit `.env`:
```env
PORT=3001
```

### Sensor readings tidak akurat
1. Lakukan kalibrasi sensor (lihat ESP32_SETUP.md)
2. Periksa wiring dan koneksi
3. Pastikan power supply stabil (5V/3.3V)

### Data tidak sampai ke server
1. Pastikan ESP32 dan server di network yang sama
2. Cek IP address server benar
3. Ping server dari network ESP32
4. Periksa firewall tidak memblokir port 3000
5. Lihat Serial Monitor ESP32 untuk response code

---

**Dibuat dengan ❤️ menggunakan Claude Code**
**© 2026 Sistem Monitoring Tanah**

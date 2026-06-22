# Setup 8-in-1 RS485 Soil Sensor

Panduan lengkap untuk mengintegrasikan sensor tanah 8-in-1 dengan komunikasi RS485/Modbus ke ESP32.

## 📋 Apa yang Diukur Sensor 8-in-1?

Sensor ini mengukur 8 parameter tanah sekaligus:
1. **Kelembaban Tanah** (Soil Moisture) - %
2. **Suhu Tanah** (Soil Temperature) - °C
3. **Konduktivitas Listrik** (EC - Electrical Conductivity) - μS/cm
4. **pH Tanah** (Soil pH) - 0-14
5. **Nitrogen** (N) - mg/kg (ppm)
6. **Fosfor** (P - Phosphorus) - mg/kg (ppm)
7. **Kalium** (K - Potassium) - mg/kg (ppm)
8. **Salinitas** (Salinity) - mg/L (ppm)

## 🔧 Hardware yang Dibutuhkan

### Komponen Utama:
- **ESP32 DevKitC** (atau compatible board)
- **8-in-1 Soil Sensor RS485** (seperti Niubol atau yang kompatibel)
- **MAX485 TTL to RS485 Converter Module**
- **Power Supply 12V** (untuk sensor - cek spesifikasi sensor Anda)
- **Power Supply 5V** (untuk ESP32 dan MAX485)
- **Breadboard & Jumper Wires**

### Optional:
- **DC-DC Buck Converter** (jika menggunakan satu power supply untuk kedua voltase)
- **Enclosure/Housing** untuk outdoor deployment

## 🔌 Wiring Diagram

### Koneksi MAX485 ke ESP32:

```
MAX485 Module          ESP32 DevKitC
─────────────          ─────────────
RO (Receiver Out)  →   GPIO 16 (RX2)
DI (Driver In)     →   GPIO 17 (TX2)
DE (Driver Enable) →   GPIO 4
RE (Rcv Enable)    →   GPIO 4  (sambung dengan DE)
VCC                →   5V
GND                →   GND
```

**PENTING:** Pin DE dan RE di MAX485 harus disambung ke GPIO yang sama (GPIO 4) untuk kontrol arah komunikasi.

### Koneksi Sensor 8-in-1 ke MAX485:

```
8-in-1 Sensor      MAX485 Module
─────────────      ─────────────
A (Yellow/Green) → A
B (Blue/White)   → B
VCC (Red)        → 12V Power Supply (+)
GND (Black)      → Common GND
```

### Diagram Lengkap:

```
                    ┌─────────────────┐
                    │  12V Power      │
                    │  Supply         │
                    └────┬────────────┘
                         │
                         ├─── VCC (Red)
                         │
┌────────────────────┐   │          ┌──────────────┐
│  8-in-1 Sensor     │   │          │   MAX485     │
│  RS485             │   │          │   Module     │
├────────────────────┤   │          ├──────────────┤
│ A (Yellow)   ──────┼───┼──────────┤ A            │
│ B (Blue)     ──────┼───┼──────────┤ B            │
│ VCC (Red)    ──────┘   │          │              │
│ GND (Black)  ──────────┼──────────┤ GND          │
└────────────────────┘   │          │ VCC    ──┐   │
                         │          │ RO     ──┼───┤ → GPIO 16
                    ┌────┴──────┐   │ DI     ──┼───┤ ← GPIO 17
                    │  ESP32    │   │ DE/RE  ──┼───┤ ← GPIO 4
                    │           │   └──────────┘   │
                    │ GND ──────┼──────────────────┘
                    │ 5V  ──────┤ (untuk MAX485)
                    └───────────┘
```

## ⚙️ Konfigurasi Sensor

### 1. Cek Konfigurasi Default Sensor

Sensor 8-in-1 biasanya memiliki konfigurasi default:
- **Slave Address (Device ID):** 0x01 (1)
- **Baud Rate:** 4800 atau 9600 bps
- **Data Format:** 8N1 (8 data bits, No parity, 1 stop bit)
- **Protocol:** Modbus RTU

**PENTING:** Cek manual sensor Anda untuk memastikan konfigurasi ini!

### 2. Setting yang Perlu Disesuaikan di Firmware

Buka file `esp32_soil_monitor/esp32_8in1_sensor.ino` dan sesuaikan:

```cpp
// Modbus Configuration
#define MODBUS_SLAVE_ID 1       // Ganti sesuai address sensor Anda
#define MODBUS_BAUDRATE 4800    // Ganti ke 9600 jika sensor menggunakan 9600

// Modbus Register Addresses
// Alamat register ini HARUS disesuaikan dengan manual sensor Anda!
#define REG_MOISTURE 0x0000      
#define REG_TEMPERATURE 0x0001   
#define REG_EC 0x0002            
#define REG_PH 0x0003            
#define REG_NITROGEN 0x0004      
#define REG_PHOSPHORUS 0x0005    
#define REG_POTASSIUM 0x0006     
#define REG_POTASSIUM 0x0007     
```

### 3. Cara Menemukan Register Address dari Manual Sensor

Lihat di manual PDF sensor (halaman Modbus Register Map), biasanya terlihat seperti:

| Register Address | Function | Unit | Resolution |
|-----------------|----------|------|------------|
| 0x0000 | Moisture | % | 0.1 |
| 0x0001 | Temperature | °C | 0.1 |
| 0x0002 | EC | μS/cm | 1 |
| ... | ... | ... | ... |

## 📝 Setup Software

### 1. Install Arduino Libraries

Buka Arduino IDE → Tools → Manage Libraries, install:

1. **ModbusMaster** by Doc Walker
   - Library untuk komunikasi Modbus RTU
   - Version: 2.0.1 atau lebih baru

2. **ArduinoJson** by Benoit Blanchon
   - Version: 6.x.x (bukan v7)

3. **WiFi** - Built-in untuk ESP32

4. **HTTPClient** - Built-in untuk ESP32

### 2. Konfigurasi WiFi dan Server

Edit di firmware `esp32_8in1_sensor.ino`:

```cpp
// WiFi Credentials
const char* ssid = "NAMA_WIFI_ANDA";
const char* password = "PASSWORD_WIFI";

// Server URL
const char* serverUrl = "http://192.168.1.XXX:3000/api/sensor-data";
```

**Cara mendapatkan IP address server:**
```bash
# Windows
ipconfig

# Linux/Mac
ifconfig
```

Cari IP address di jaringan lokal (biasanya 192.168.x.x atau 10.0.x.x).

### 3. Upload Firmware

1. Hubungkan ESP32 ke komputer via USB
2. Pilih board: **Tools → Board → ESP32 Dev Module**
3. Pilih port: **Tools → Port → COM X** (Windows) atau **/dev/ttyUSB0** (Linux)
4. Klik **Upload** (ikon panah kanan)
5. Tunggu hingga selesai (± 30 detik)

### 4. Monitor Serial Output

1. Buka **Tools → Serial Monitor**
2. Set baud rate ke **115200**
3. Anda akan melihat output seperti:

```
===========================================
ESP32 Soil Monitoring - 8-in-1 RS485 Sensor
===========================================

✓ RS485 direction control configured
✓ RS485 UART initialized at 4800 baud
✓ Modbus initialized (Slave ID: 1)

Connecting to WiFi: YourWiFiName
..................
✓ WiFi Connected!
IP Address: 192.168.1.150
Signal Strength (RSSI): -45 dBm

Testing sensor connection...
Reading from 8-in-1 sensor...
✓ All sensor values read successfully
✓ Sensor communication successful!

✓ System ready!
Starting sensor readings...

========== Sensor Readings ==========
--- Soil Conditions ---
Moisture:       45.2 %
Temperature:    24.5 °C
EC:             850 μS/cm
pH:             6.75

--- NPK Nutrients ---
Nitrogen (N):   85 mg/kg
Phosphorus (P): 42 mg/kg
Potassium (K):  120 mg/kg

--- Other ---
Salinity:       320 mg/L
=====================================
```

## 🐛 Troubleshooting

### Sensor tidak terbaca (Error reading)

**Masalah:** Serial Monitor menampilkan error seperti:
```
✗ Failed to read moisture. Error: E2
```

**Solusi:**
1. **Cek wiring A dan B:**
   - Pastikan tidak terbalik
   - Coba tukar A dan B jika masih error

2. **Cek power supply sensor:**
   - Sensor 8-in-1 butuh 12V (atau sesuai spesifikasi)
   - Gunakan multimeter untuk verifikasi voltage

3. **Cek baud rate:**
   - Coba ganti dari 4800 ke 9600 atau sebaliknya
   - Restart ESP32 setelah perubahan

4. **Cek Slave ID:**
   - Default biasanya 1, tapi bisa berbeda
   - Coba ID 1, 2, atau 0

5. **Cek common ground:**
   - Semua GND harus terhubung: ESP32, MAX485, Sensor, Power Supply

### WiFi tidak connect

**Solusi:**
- Gunakan WiFi 2.4 GHz (ESP32 tidak support 5 GHz)
- Cek SSID dan password benar
- Dekatkan ESP32 ke router

### Data tidak sampai ke server

**Cek:**
1. Server backend running di `http://localhost:3000`
2. IP address server benar di firmware
3. Firewall tidak memblokir port 3000
4. ESP32 dan server dalam satu jaringan

**Test koneksi:**
```bash
# Dari komputer yang sama dengan server
curl http://localhost:3000/api/health

# Dari ESP32 network
ping 192.168.1.XXX
```

### Pembacaan sensor tidak stabil

**Solusi:**
1. Tambahkan delay antara pembacaan Modbus:
   ```cpp
   delay(200);  // Ganti dari 100 ke 200ms
   ```

2. Cek kabel RS485:
   - Gunakan twisted pair cable untuk jarak jauh
   - Maksimal 1200m untuk RS485

3. Tambahkan termination resistor 120Ω di ujung line (untuk instalasi panjang)

## 📊 Memahami Data Sensor

### Soil Moisture (Kelembaban)
- **Range:** 0-100%
- **Ideal untuk tanaman:** 40-80%
- **< 30%:** Tanah kering, perlu penyiraman
- **> 80%:** Tanah terlalu basah, bahaya akar busuk

### Soil Temperature (Suhu)
- **Range:** -40°C to 80°C
- **Ideal untuk kebanyakan tanaman:** 20-30°C
- **< 15°C:** Pertumbuhan lambat
- **> 35°C:** Stress pada akar

### EC (Electrical Conductivity)
- **Range:** 0-20,000 μS/cm
- **0-800:** Low salinity (cocok untuk tanaman sensitif)
- **800-2000:** Medium (cocok untuk kebanyakan tanaman)
- **> 2000:** High (bisa toxic untuk tanaman tertentu)

### pH (Keasaman)
- **Range:** 0-14
- **Ideal untuk kebanyakan tanaman:** 6.0-7.5
- **< 6.0:** Asam (cocok: blueberry, azalea)
- **> 7.5:** Basa/alkaline

### NPK (Nitrogen, Phosphorus, Potassium)
Nilai dalam mg/kg (ppm):

**Nitrogen (N):**
- Low: < 50 ppm
- Medium: 50-100 ppm
- High: > 100 ppm

**Phosphorus (P):**
- Low: < 20 ppm
- Medium: 20-50 ppm
- High: > 50 ppm

**Potassium (K):**
- Low: < 100 ppm
- Medium: 100-200 ppm
- High: > 200 ppm

### Salinity (Salinitas)
- **Range:** 0-10,000 mg/L
- **< 500:** Low (safe untuk semua tanaman)
- **500-1500:** Medium
- **> 1500:** High (berbahaya untuk banyak tanaman)

## 🔄 Next Steps

Setelah sensor berfungsi:

1. ✅ **Kalibrasi** - Bandingkan dengan soil test kit profesional
2. ✅ **Set threshold** - Sesuaikan alarm/notification sesuai tanaman
3. ✅ **Update backend** - Gunakan backend yang sudah diupdate untuk 8 parameter
4. ✅ **Update dashboard** - Lihat semua 8 parameter di web dashboard

## 📚 Referensi

- **Modbus Protocol:** https://www.modbustools.com/modbus.html
- **MAX485 Datasheet:** https://www.ti.com/lit/ds/symlink/max485.pdf
- **ESP32 UART:** https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html
- **Soil NPK Testing:** https://extension.psu.edu/soil-fertility-testing

---

**Butuh bantuan?** Buka issue atau hubungi untuk support teknis.

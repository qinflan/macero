# Macero

**A Powerful ESP32-Based Security Research Platform**

Macero is a sophisticated penetration testing and security research tool built on the ESP32 microcontroller platform. Designed with both educational and professional security research in mind, this project demonstrates advanced wireless exploitation techniques while maintaining a focus on ethical hacking principles.

Originally developed as a Computer Architecture course project, Macero has evolved into a comprehensive security toolkit that showcases the capabilities of embedded systems in offensive security research.

---

## Legal Disclaimer

**IMPORTANT:** This tool is designed exclusively for:

- Authorized penetration testing
- Security research in controlled environments
- Educational purposes with proper consent
- Testing your own networks and devices

**Unauthorized access to computer networks is illegal.** Users are solely responsible for ensuring they have explicit permission before deploying any of Macero's features. The developers assume no liability for misuse of this software.

---

## Features

### Evil Captive Portal

Launch a sophisticated phishing attack by creating a fake WiFi access point with a captive portal that mimics popular login pages.

- **Automated DNS hijacking** - Redirects all DNS queries to the captive portal
- **Custom web interface** - Serves convincing login pages to harvest credentials
- **Data harvesting & storage** - Captured credentials are stored and viewable on the OLED display
- **Real-time monitoring** - View captured data through an intuitive menu interface

### WiFi Beacon Flooding

Overwhelm nearby devices with a flood of fake WiFi networks, causing confusion and denial-of-service effects.

- **Randomized SSIDs** - Generates hundreds of fake network names
- **MAC address spoofing** - Each beacon uses a randomized MAC address
- **Adjustable intensity** - Configurable packet transmission rates
- **Network saturation** - Effectively clutters WiFi scanners and causes interference

### BLE Advertisement Spam

Exploit Bluetooth Low Energy by flooding the area with fake device advertisements.

- **Device spoofing** - Mimics popular Bluetooth devices (Apple, Samsung, etc.)
- **Continuous broadcasting** - Rapidly cycles through advertisements
- **Proximity attacks** - Causes notification spam on nearby smartphones
- **Protocol exploitation** - Leverages BLE advertising mechanics for maximum impact

### Interactive UI

Navigate features through an intuitive menu system displayed on the SSD1306 OLED screen.

- **Multi-level menu structure** - Organized attack modes and settings
- **Three-button navigation** - Simple and responsive control scheme
- **Real-time status updates** - Visual feedback for all operations
- **LED indicators** - Visual confirmation of active attack modes

---

## Hardware Requirements

### Core Components

- **ESP32 WROOM-32 DevKit V1** - Main microcontroller board
  - Dual-core Xtensa processor @ 240MHz
  - Built-in WiFi 802.11 b/g/n
  - Bluetooth 4.2 (BR/EDR + BLE)
  - 520KB SRAM

### Peripherals

- **SSD1306 OLED Display** (128x64 pixels, I2C interface)
  - Used for menu navigation and status display
  - Typical pinout: SCL → GPIO22, SDA → GPIO21

- **3x Tactile Push Buttons**
  - GPIO26 - Button 1 (Navigate Up/Select)
  - GPIO27 - Button 2 (Navigate Down/Back)
  - GPIO25 - Button 3 (Confirm/Action)

- **1x Status LED** (optional but recommended)
  - GPIO2 - Visual indicator for active operations

### Optional Components

- Breadboard or custom PCB for cleaner assembly
- External antenna for improved WiFi/BLE range
- Battery pack for portable operation (ensure adequate power supply)

---

## Installation

### Prerequisites

**ESP-IDF v5.4** is required to build and flash this firmware. Follow the official [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32/get-started/index.html) for your operating system.

#### Quick Setup (Linux/macOS)

```bash
# Install ESP-IDF
mkdir -p ~/esp
cd ~/esp
git clone -b v5.4 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32
. ./export.sh
```

#### Quick Setup (Windows)

Download and run the [ESP-IDF Windows Installer](https://dl.espressif.com/dl/esp-idf/) for v5.4.

### Building & Flashing

1. **Clone the repository**

   ```bash
   git clone <your-repo-url>
   cd macero/software
   ```

2. **Configure the project** (optional)

   ```bash
   idf.py menuconfig
   ```

   Adjust WiFi settings, GPIO pins, or other parameters as needed.

3. **Build the firmware**

   ```bash
   idf.py build
   ```

4. **Connect your ESP32** via USB and flash

   ```bash
   idf.py -p COM3 flash monitor  # Windows
   # or
   idf.py -p /dev/ttyUSB0 flash monitor  # Linux/macOS
   ```

   Replace the port with your device's actual port.

5. **Monitor output** (included in the command above)
   - View real-time logs and debug information
   - Press `Ctrl+]` to exit the monitor

---

## Usage

### Navigation

- **Button 1 (GPIO26)** - Scroll up through menu options
- **Button 2 (GPIO27)** - Scroll down through menu options
- **Button 3 (GPIO25)** - Select/Execute current option

### Menu Structure

```
Main Menu
├── WiFi Settings
│   ├── Evil Captive Portal
│   │   ├── Start Portal
│   │   └── View Harvested Data
│   └── Beacon Flood
│       ├── Start Flood
│       └── Stop Flood
└── Bluetooth Settings
    └── BLE Advertisement Spam
        ├── Start BLE Spam
        └── Stop BLE Spam
```

### Launching Attacks

#### Evil Captive Portal

1. Navigate to `WiFi Settings` → `Evil Captive Portal`
2. Select `Start Portal`
3. The ESP32 will create a fake AP (default SSID can be configured)
4. Victims connecting to the AP will be redirected to a login page
5. View captured credentials under `View Harvested Data`

#### WiFi Beacon Flooding

1. Navigate to `WiFi Settings` → `Beacon Flood`
2. Select `Start Flood`
3. The device will begin broadcasting hundreds of fake networks
4. Select `Stop Flood` to terminate the attack

#### BLE Advertisement Spam

1. Navigate to `Bluetooth Settings` → `BLE Advertisement Spam`
2. Select `Start BLE Spam`
3. Nearby devices will receive constant BLE notifications
4. Select `Stop BLE Spam` to cease broadcasting

---

## Technical Architecture

### Software Components

- **FreeRTOS** - Real-time operating system for task management
- **ESP-IDF Framework** - Official Espressif IoT Development Framework
- **lwIP** - Lightweight TCP/IP stack for network operations
- **Custom DNS Server** - Intercepts and redirects DNS queries
- **HTTP Server** - Serves phishing pages and handles POST requests
- **SSD1306 Driver** - Custom display driver for OLED interface

### Key Modules

| Module         | Purpose                           | Key Files                              |
| -------------- | --------------------------------- | -------------------------------------- |
| Captive Portal | Evil AP + DNS + HTTP server       | `captive_portal.c/h`, `dns_server.c/h` |
| Beacon Flood   | WiFi frame injection              | `beacon_flood.c/h`                     |
| BLE Spam       | Bluetooth advertisement flooding  | `ble.c/h`                              |
| UI System      | Menu navigation and display       | `macero.c/h`, `ssd1306` component      |
| Utilities      | Helper functions and common logic | `utils.c/h`                            |

### Memory Management

The project leverages FreeRTOS heap management and carefully manages WiFi/BLE buffers to ensure stable operation on the resource-constrained ESP32 platform.

---

## Roadmap & Future Features

### Planned Features

- [ ] **HID Keystroke Injection** - USB Rubber Ducky-style attacks via BLE HID
- [ ] **BLE Device Scanning & Cloning** - Enumerate and impersonate nearby BLE devices
- [ ] **WiFi Deauthentication Attacks** - Forcibly disconnect clients from networks
- [ ] **Packet Capture** - Store intercepted network traffic to SD card
- [ ] **WPA2 Handshake Capture** - Collect handshakes for offline cracking
- [ ] **Web-Based Configuration** - Configure attacks via smartphone/computer
- [ ] **SD Card Support** - Store larger datasets and logs
- [ ] **GPS Integration** - Log attack locations for wardriving applications

### Enhancements

- Improved OLED UI with icons and animations
- Battery level monitoring
- Over-the-air (OTA) firmware updates
- Multi-language captive portal templates
- Custom portal HTML template editor

---

## Contributing

Contributions are welcome! Whether you're fixing bugs, adding features, or improving documentation, your help is appreciated.

### How to Contribute

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Guidelines

- Follow existing code style and conventions
- Comment complex logic and algorithms
- Test thoroughly on actual hardware before submitting
- Update documentation to reflect changes

---

## License

This project is provided for educational and research purposes. Users are responsible for complying with all applicable laws and regulations in their jurisdiction.

---

## Acknowledgments

- **Espressif Systems** - For the excellent ESP-IDF framework and documentation
- **FreeRTOS** - For the reliable real-time operating system
- **SSD1306 Community** - For OLED display drivers and resources
- **Security Research Community** - For inspiration and shared knowledge

---

## Additional Resources

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [ESP32 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
- [WiFi 802.11 Frame Specification](https://en.wikipedia.org/wiki/IEEE_802.11)
- [Bluetooth Core Specification](https://www.bluetooth.com/specifications/specs/)

---

**Built with passion for security research**

_Remember: With great power comes great responsibility. Use Macero ethically._

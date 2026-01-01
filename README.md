# 🌌 **AERO-CORE: Intelligent Thermal Management System** 🚀

![License](https://img.shields.io/badge/license-MIT-blue.svg) ![Platform](https://img.shields.io/badge/platform-STM32-orange.svg) ![Framework](https://img.shields.io/badge/framework-STM32Cube-green.svg) ![Status](https://img.shields.io/badge/status-OPERATIONAL-brightgreen.svg)

> *Precision Cooling. Absolute Reliability. Future-Ready Interface.*

---

## 📡 **Mission Overview**

Welcome to **AERO-CORE**, the next-generation firmware solution for the **STM32F103C8T6 (Blue Pill)**. This project transforms your microcontroller into an autonomous, safety-critical thermal regulation unit. Designed for high-performance cooling applications, it leverages advanced PWM modulation, non-volatile memory, and a robust fail-safe architecture to ensure your hardware stays cool under pressure.

Whether you are cooling a server rack, a 3D printer, or a fusion reactor (experimental), AERO-CORE delivers linear precision and instant adaptability.

---

## ⚡ **Core Modules**

### 🌀 **Adaptive PWM Airflow Control**
Utilizing **TIM2** hardware timers, the system drives a low-side MOSFET to modulate fan speed with 1000-step resolution.
*   **Idle State**: 0% Speed (Below Min Temp)
*   **Linear Thrust**: Smooth interpolation between Min and Max thresholds.
*   **Max Override**: 100% Speed (Above Max Temp)

### 🛡️ **Fail-Safe Sentinel**
Safety is paramount. The **Watchdog (IWDG)** continuously monitors system health.
*   **Sensor Breach Protocol**: If the DS18B20 sensor data line is severed or corrupted, the system immediately engages **Emergency Override**, locking the fan at **100%** capacity to prevent thermal runaway.

### 💾 **NVM Persistence Layer**
Configuration is stored in emulated EEPROM (Flash Memory). Your custom temperature thresholds survive power cycles, ensuring the mission profile is always loaded on boot.

### 📟 **Cyber-Link CLI**
Interact with the core via the Serial Command Line Interface. No drivers needed—just pure ASCII control.

---

## 🔌 **Hardware Interface (Pinout)**

| Component | Pin | Function | Notes |
| :--- | :--- | :--- | :--- |
| **Fan Control** | `PA1` | PWM Output | Connect to Gate of N-Ch MOSFET |
| **Thermal Sensor** | `PA0` | 1-Wire Data | DS18B20 (Requires 4.7k Pull-up) |
| **Console TX** | `PA9` | UART TX | Connect to USB-TTL RX |
| **Console RX** | `PA10`| UART RX | Connect to USB-TTL TX |
| **Heartbeat** | `PC13`| Status LED | Pulses @ 1Hz |

---

## ⌨️ **Command Manual**

Connect via Serial Terminal (Baud: `115200`, 8N1).

| Command | Parameter | Description |
| :--- | :--- | :--- |
| `help` | - | Display valid directives. |
| `get_temp` | - | Retrieve current core temperature (°C). |
| `get_settings` | - | Display active Min/Max thresholds. |
| `set_min` | `<value>` | Set activation threshold (e.g., `set_min 30.0`). |
| `set_max` | `<value>` | Set max-thrust threshold (e.g., `set_max 60.0`). |

> **Example Session:**
> ```bash
> > get_temp
> Current Temp: 34.50
> > set_min 40.0
> Min temp set to: 40.00
> ```

---

## 🛠️ **Deployment Instructions**

### Prerequisites
*   **PlatformIO** installed on your workstation.
*   **STM32F103C8T6** ("Blue Pill") board.
*   **ST-Link V2** or compatible programmer.

### Initialization & Compilation
Initialize the neural pathways (build the code):

```bash
# Clone the repository
git clone <repo-url>
cd <repo-name>

# Build the firmware
pio run
```

### Flash the Core
Upload the firmware to the target unit:

```bash
pio run --target upload
```

---

## 🔮 **Future Roadmap**

*   [ ] PID Control Loop implementation.
*   [ ] OLED Display integration (I2C).
*   [ ] Remote Telemetry via ESP8266 bridge.

---

*Engineered by Jules.* 🤖

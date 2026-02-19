import sys
import time
from dataclasses import dataclass

from PySide6.QtCore import Qt, QTimer
from PySide6.QtGui import QColor, QFont
from PySide6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton,
    QComboBox, QSpinBox, QLineEdit, QTextEdit, QGroupBox, QColorDialog,
    QMessageBox
)

import serial
import serial.tools.list_ports


@dataclass
class RGB:
    r: int
    g: int
    b: int

    def clamp(self) -> "RGB":
        return RGB(
            max(0, min(255, int(self.r))),
            max(0, min(255, int(self.g))),
            max(0, min(255, int(self.b))),
        )


class SerialClient:
    def __init__(self):
        self.ser: serial.Serial | None = None

    def is_open(self) -> bool:
        return self.ser is not None and self.ser.is_open

    def connect(self, port: str, baud: int = 115200) -> None:
        self.disconnect()
        self.ser = serial.Serial(port, baudrate=baud, timeout=0.05)
        # ESP8266 часто ресетится при открытии порта, подождём немного
        time.sleep(1.2)
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

    def disconnect(self) -> None:
        if self.ser is not None:
            try:
                self.ser.close()
            except Exception:
                pass
        self.ser = None

    def send_line(self, line: str) -> None:
        if not self.is_open():
            raise RuntimeError("Serial not connected")
        if not line.endswith("\n"):
            line += "\n"
        self.ser.write(line.encode("utf-8", errors="replace"))

    def read_available(self) -> str:
        if not self.is_open():
            return ""
        try:
            data = self.ser.read(4096)
            return data.decode("utf-8", errors="replace")
        except Exception:
            return ""


def list_com_ports() -> list[str]:
    ports = []
    for p in serial.tools.list_ports.comports():
        # пример: "COM7 - USB-SERIAL CH340"
        ports.append(p.device)
    return ports


class ColorButton(QPushButton):
    def __init__(self, title: str, initial: QColor):
        super().__init__(title)
        self._color = initial
        self.setCursor(Qt.PointingHandCursor)
        self.setFixedHeight(34)
        self.update_style()

    @property
    def color(self) -> QColor:
        return self._color

    def set_color(self, c: QColor):
        if c.isValid():
            self._color = c
            self.update_style()

    def update_style(self):
        c = self._color
        # контрастный текст
        luminance = 0.2126 * c.red() + 0.7152 * c.green() + 0.0722 * c.blue()
        fg = "#111111" if luminance > 150 else "#F2F2F2"
        self.setStyleSheet(
            f"""
            QPushButton {{
                background-color: rgb({c.red()}, {c.green()}, {c.blue()});
                color: {fg};
                border: 1px solid rgba(0,0,0,0.2);
                border-radius: 10px;
                padding: 6px 10px;
                font-weight: 600;
            }}
            QPushButton:hover {{
                border: 1px solid rgba(0,0,0,0.4);
            }}
            """
        )


class App(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("ESP8266 TFT Controller (Serial)")
        self.setMinimumWidth(720)

        self.client = SerialClient()

        root = QVBoxLayout(self)
        root.setSpacing(12)

        # --- Top: connection ---
        conn_box = QGroupBox("Подключение")
        conn_l = QHBoxLayout(conn_box)

        self.port_cb = QComboBox()
        self.refresh_ports()

        self.baud_cb = QComboBox()
        self.baud_cb.addItems(["115200", "57600", "9600"])
        self.baud_cb.setCurrentText("115200")

        self.btn_refresh = QPushButton("Обновить порты")
        self.btn_connect = QPushButton("Подключить")
        self.btn_disconnect = QPushButton("Отключить")
        self.btn_disconnect.setEnabled(False)

        for b in (self.btn_refresh, self.btn_connect, self.btn_disconnect):
            b.setCursor(Qt.PointingHandCursor)
            b.setFixedHeight(34)

        conn_l.addWidget(QLabel("COM:"))
        conn_l.addWidget(self.port_cb, 2)
        conn_l.addWidget(QLabel("Baud:"))
        conn_l.addWidget(self.baud_cb, 1)
        conn_l.addWidget(self.btn_refresh)
        conn_l.addWidget(self.btn_connect)
        conn_l.addWidget(self.btn_disconnect)

        root.addWidget(conn_box)

        # --- Controls: Fill + Text ---
        controls = QHBoxLayout()
        controls.setSpacing(12)

        # Fill group
        fill_box = QGroupBox("Фон (FILL)")
        fill_l = QVBoxLayout(fill_box)

        self.bg_btn = ColorButton("Выбрать цвет фона", QColor(0, 0, 0))
        self.btn_fill = QPushButton("Залить экран")
        self.btn_clear = QPushButton("Очистить (чёрный)")

        for b in (self.btn_fill, self.btn_clear):
            b.setCursor(Qt.PointingHandCursor)
            b.setFixedHeight(34)

        fill_l.addWidget(self.bg_btn)
        fill_l.addWidget(self.btn_fill)
        fill_l.addWidget(self.btn_clear)
        fill_l.addStretch(1)

        # Text group
        text_box = QGroupBox("Текст (TEXT)")
        text_l = QVBoxLayout(text_box)

        row1 = QHBoxLayout()
        self.x_spin = QSpinBox(); self.x_spin.setRange(0, 200); self.x_spin.setValue(10)
        self.y_spin = QSpinBox(); self.y_spin.setRange(0, 200); self.y_spin.setValue(30)
        self.size_spin = QSpinBox(); self.size_spin.setRange(1, 6); self.size_spin.setValue(2)
        row1.addWidget(QLabel("X:")); row1.addWidget(self.x_spin)
        row1.addWidget(QLabel("Y:")); row1.addWidget(self.y_spin)
        row1.addWidget(QLabel("Size:")); row1.addWidget(self.size_spin)

        self.txt_btn = ColorButton("Цвет текста", QColor(255, 255, 0))
        self.msg_edit = QLineEdit("Hello World!")
        self.msg_edit.setPlaceholderText("Введите текст…")

        self.btn_send_text = QPushButton("Печатать текст")
        self.btn_send_text.setCursor(Qt.PointingHandCursor)
        self.btn_send_text.setFixedHeight(34)

        text_l.addLayout(row1)
        text_l.addWidget(self.txt_btn)
        text_l.addWidget(QLabel("Сообщение:"))
        text_l.addWidget(self.msg_edit)
        text_l.addWidget(self.btn_send_text)
        text_l.addStretch(1)

        controls.addWidget(fill_box, 1)
        controls.addWidget(text_box, 2)
        root.addLayout(controls)

        # --- Log ---
        log_box = QGroupBox("Лог / ответ ESP")
        log_l = QVBoxLayout(log_box)
        self.log = QTextEdit()
        self.log.setReadOnly(True)
        self.log.setFont(QFont("Consolas", 10))
        log_l.addWidget(self.log)
        root.addWidget(log_box)

        # --- wiring ---
        self.btn_refresh.clicked.connect(self.refresh_ports)
        self.btn_connect.clicked.connect(self.on_connect)
        self.btn_disconnect.clicked.connect(self.on_disconnect)

        self.bg_btn.clicked.connect(lambda: self.pick_color(self.bg_btn))
        self.txt_btn.clicked.connect(lambda: self.pick_color(self.txt_btn))

        self.btn_fill.clicked.connect(self.on_fill)
        self.btn_clear.clicked.connect(self.on_clear)
        self.btn_send_text.clicked.connect(self.on_text)

        # Poll ESP serial output
        self.timer = QTimer(self)
        self.timer.setInterval(80)
        self.timer.timeout.connect(self.poll_serial)
        self.timer.start()

        self.set_enabled_controls(False)

        self.append_log("Готово. Выбери COM-порт, нажми «Подключить».\n")

    def append_log(self, s: str):
        self.log.append(s.rstrip("\n"))

    def refresh_ports(self):
        current = self.port_cb.currentText()
        self.port_cb.clear()
        ports = list_com_ports()
        self.port_cb.addItems(ports)
        if current in ports:
            self.port_cb.setCurrentText(current)

    def set_enabled_controls(self, enabled: bool):
        self.bg_btn.setEnabled(enabled)
        self.btn_fill.setEnabled(enabled)
        self.btn_clear.setEnabled(enabled)
        self.x_spin.setEnabled(enabled)
        self.y_spin.setEnabled(enabled)
        self.size_spin.setEnabled(enabled)
        self.txt_btn.setEnabled(enabled)
        self.msg_edit.setEnabled(enabled)
        self.btn_send_text.setEnabled(enabled)

    def pick_color(self, btn: ColorButton):
        c = QColorDialog.getColor(btn.color, self, "Выбор цвета")
        if c.isValid():
            btn.set_color(c)

    def on_connect(self):
        port = self.port_cb.currentText().strip()
        if not port:
            QMessageBox.warning(self, "COM не выбран", "Выбери COM-порт и попробуй снова.")
            return
        baud = int(self.baud_cb.currentText())
        try:
            self.client.connect(port, baud)
        except Exception as e:
            QMessageBox.critical(self, "Не удалось подключиться", str(e))
            return

        self.append_log(f"✅ Connected: {port} @ {baud}\n")
        self.btn_connect.setEnabled(False)
        self.btn_disconnect.setEnabled(True)
        self.btn_refresh.setEnabled(False)
        self.set_enabled_controls(True)

    def on_disconnect(self):
        self.client.disconnect()
        self.append_log("⛔ Disconnected\n")
        self.btn_connect.setEnabled(True)
        self.btn_disconnect.setEnabled(False)
        self.btn_refresh.setEnabled(True)
        self.set_enabled_controls(False)

    def on_fill(self):
        c = self.bg_btn.color
        cmd = f"FILL {c.red()} {c.green()} {c.blue()}"
        self.send(cmd)

    def on_clear(self):
        self.send("FILL 0 0 0")

    def on_text(self):
        msg = self.msg_edit.text().strip()
        if not msg:
            return
        x = self.x_spin.value()
        y = self.y_spin.value()
        size = self.size_spin.value()
        c = self.txt_btn.color
        # ВАЖНО: пробелы в тексте поддерживаются (мы шлём как есть)
        cmd = f"TEXT {x} {y} {size} {c.red()} {c.green()} {c.blue()} {msg}"
        self.send(cmd)

    def send(self, cmd: str):
        if not self.client.is_open():
            QMessageBox.warning(self, "Нет соединения", "Сначала подключись к COM-порту.")
            return
        try:
            self.client.send_line(cmd)
            self.append_log(f"> {cmd}\n")
        except Exception as e:
            QMessageBox.critical(self, "Ошибка отправки", str(e))

    def poll_serial(self):
        if not self.client.is_open():
            return
        out = self.client.read_available()
        if out:
            self.append_log(out)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    w = App()
    w.show()
    sys.exit(app.exec())

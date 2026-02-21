---

# QR for TFT (ESP8266 + ST7735): генерация 1-bit BMP 128×128

В этом проекте QR-коды показываются на TFT как **готовые картинки BMP** с SD-карты.  
Чтобы QR корректно отображался и считывался камерой, используем единый формат:

✅ **128×128 пикселей**  
✅ **1-bit (монохром)**  
✅ **BMP без сжатия**  
✅ С обязательным **quiet zone** (рамка-поля) в модулях QR

> Почему не 512×512?  
> На ST7735 (160×128) большой QR будет отображаться частично (виден будет только позиционный квадрат), поэтому файлы готовим сразу под размер экрана.

---

## 1) Требования

- ПК (Windows / macOS / Linux)
- Python 3.9+ (рекомендуется)
- (опционально) ImageMagick, если вы конвертируете уже готовые PNG/SVG

---

## 2) Установка (Python)

### 2.1 Установка Python-пакетов
```bash
pip install qrcode[pil]
````

Проверка:

```bash
python -c "import qrcode; print('qrcode ok')"
```

---

## 3) Генерация QR → BMP 128×128 (1-bit)

Создайте файл `make_qr_bmp128.py`:

```python
import argparse
import qrcode
from PIL import Image

def make_qr_bmp128(data: str, out: str, size: int = 128, border: int = 4,
                   ec: str = "M", invert: bool = False):
    ec_map = {
        "L": qrcode.constants.ERROR_CORRECT_L,
        "M": qrcode.constants.ERROR_CORRECT_M,
        "Q": qrcode.constants.ERROR_CORRECT_Q,
        "H": qrcode.constants.ERROR_CORRECT_H,
    }
    if ec not in ec_map:
        raise SystemExit("EC must be one of: L, M, Q, H")

    qr = qrcode.QRCode(
        version=None,              # авто-версия
        error_correction=ec_map[ec],
        box_size=1,
        border=border              # quiet zone (обычно 4)
    )
    qr.add_data(data)
    qr.make(fit=True)

    # Строим QR: чёрное=0, белое=255 (для 1-bit)
    if not invert:
        img = qr.make_image(fill_color=0, back_color=255).convert("1")
    else:
        img = qr.make_image(fill_color=255, back_color=0).convert("1")

    # Ровно 128×128 без размытия
    img = img.resize((size, size), resample=Image.NEAREST).convert("1")

    # Сохраняем в BMP (mode "1" -> 1bpp BMP)
    img.save(out, format="BMP")
    print("Saved:", out)

def main():
    ap = argparse.ArgumentParser(
        description="Generate 1-bit QR BMP for ST7735 (128x128 by default)"
    )
    ap.add_argument("--data", required=True, help="QR payload (text/url/WIFI:... string)")
    ap.add_argument("--out", required=True, help="Output BMP filename, e.g. qr_web_128.bmp")
    ap.add_argument("--size", type=int, default=128, help="Output size (default 128)")
    ap.add_argument("--border", type=int, default=4, help="Quiet zone border in modules (default 4)")
    ap.add_argument("--ec", default="M", choices=["L","M","Q","H"], help="Error correction (default M)")
    ap.add_argument("--invert", action="store_true", help="Invert colors (white QR on black)")
    args = ap.parse_args()

    make_qr_bmp128(
        data=args.data,
        out=args.out,
        size=args.size,
        border=args.border,
        ec=args.ec,
        invert=args.invert,
    )

if __name__ == "__main__":
    main()
```

---

## 4) Примеры использования (параметры)

### 4.1 QR со ссылкой на веб-интерфейс устройства

```bash
python make_qr_bmp128.py --data "http://192.168.1.68" --out qr_web_128.bmp
```

### 4.2 QR на API endpoint

```bash
python make_qr_bmp128.py --data "http://192.168.1.68/api" --out qr_api_128.bmp
```

### 4.3 QR для скачивания файла с SD (пример)

```bash
python make_qr_bmp128.py --data "http://192.168.1.68/sd?path=/qr/qr_web_128.bmp" --out qr_sdfile_128.bmp
```

### 4.4 QR для Wi-Fi (подключение без ввода пароля)

Формат:
`WIFI:T:<тип>;S:<SSID>;P:<пароль>;;`

Пример:

```bash
python make_qr_bmp128.py --data "WIFI:T:WPA;S:MySSID;P:MyPassword;;" --out qr_wifi_128.bmp
```

### 4.5 Увеличить устойчивость (коррекция ошибок Q/H)

Если строка длинная или нужен “живучий” QR:

```bash
python make_qr_bmp128.py --data "http://192.168.1.68" --out qr_web_128.bmp --ec Q
```

### 4.6 Уменьшить/увеличить quiet zone

> Обычно **border=4** — лучший выбор.
> Если сканируется плохо — НЕ уменьшайте border, лучше наоборот увеличить.

```bash
python make_qr_bmp128.py --data "http://192.168.1.68" --out qr_web_128.bmp --border 5
```

---

## 5) Куда класть QR на SD

Рекомендуемая структура:

```
/qr/
  qr_web_128.bmp
  qr_api_128.bmp
  qr_wifi_128.bmp
```

> Имя файла лучше делать “говорящим”.
> Важно: расширение `.bmp` маленькими/большими — как у вас принято в проекте (SD библиотека обычно нормально, но лучше единообразно).

---

## 6) Как пользоваться на устройстве

1. Скопируйте BMP в папку `/qr/` на SD
2. Включите устройство
3. Откройте в веб-интерфейсе/через API показ нужного файла (как у вас реализовано)
4. Наведите камеру смартфона на экран — QR должен считаться

---

## 7) Типичные ошибки и как их избежать

### ❌ QR 512×512 или больше

На TFT 160×128 будет виден только угол (позиционный квадрат), QR не считается.
✅ Делайте 128×128.

### ❌ Не 1-bit / палитра / серый

Серые пиксели, антиалиасинг и “полутона” ухудшают распознавание.
✅ Только ч/б (1-bit).

### ❌ Нет quiet zone (border)

QR без полей часто не считывается.
✅ border минимум 4.

### ❌ Длинный текст в QR

Чем больше данных — тем мельче модули, тем хуже сканируется на маленьком дисплее.
✅ Лучше: короткий URL на страницу/endpoint.

---

## 8) Альтернатива: ImageMagick (если QR уже есть как PNG)

Если у вас уже есть `input.png`, можно конвертировать:

```bash
magick input.png -resize 128x128\! -threshold 50% -monochrome BMP3:qr_128.bmp
```

---

## 9) Рекомендованные “шаблоны” payload

* URL на устройство:
  `http://192.168.1.68`
* URL на локальный домен (если используете mDNS):
  `http://device.local`
* API endpoint:
  `http://192.168.1.68/api`
* Файл на SD:
  `http://192.168.1.68/sd?path=/qr/qr_web_128.bmp`
* Wi-Fi:
  `WIFI:T:WPA;S:MySSID;P:MyPassword;;`

---

## 10) Лицензия / заметки

Скрипт генерации QR — утилита для подготовки ассетов под TFT.
Поддерживайте единый формат ассетов, чтобы вывод на дисплей был предсказуемым.

```



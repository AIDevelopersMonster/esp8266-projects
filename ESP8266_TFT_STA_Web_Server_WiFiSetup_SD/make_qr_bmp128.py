import argparse
import qrcode
from PIL import Image

EC = {
    "L": qrcode.constants.ERROR_CORRECT_L,
    "M": qrcode.constants.ERROR_CORRECT_M,
    "Q": qrcode.constants.ERROR_CORRECT_Q,
    "H": qrcode.constants.ERROR_CORRECT_H,
}

def build(data: str, out: str, target: int = 128, border: int = 4, ec: str = "M"):
    qr = qrcode.QRCode(
        version=None,
        error_correction=EC[ec],
        box_size=1,
        border=border
    )
    qr.add_data(data)
    qr.make(fit=True)

    # Чистый фон/черный без “серого”
    img = qr.make_image(fill_color=(0,0,0), back_color=(255,255,255)).convert("L")

    # Threshold -> 1-bit без dithering
    bw = img.point(lambda p: 0 if p < 128 else 255, mode="1")

    n = bw.size[0]
    scale = target // n
    scaled = bw.resize((n*scale, n*scale), Image.NEAREST)

    canvas = Image.new("1", (target, target), 1)  # white
    x = (target - scaled.size[0]) // 2
    y = (target - scaled.size[1]) // 2
    canvas.paste(scaled, (x, y))
    canvas.save(out, format="BMP")
    print("Saved:", out, "| n=", n, "scale=", scale, "border=", border, "ec=", ec)

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--data", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--target", type=int, default=128)
    ap.add_argument("--border", type=int, default=4)
    ap.add_argument("--ec", choices=["L","M","Q","H"], default="M")
    args = ap.parse_args()
    build(args.data, args.out, args.target, args.border, args.ec)
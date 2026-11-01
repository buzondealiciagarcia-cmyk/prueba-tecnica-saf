"""Presentación PowerPoint 16:9 con la paleta STD-UI desde presentacion.json."""
from pathlib import Path

from . import common as c

W_IN, H_IN = 13.333, 7.5
LAYOUTS = ("titulo", "seccion", "vinetas", "dos_columnas", "imagen", "tabla", "kpis", "cierre")


def validate(spec):
    errs = []
    slides = spec.get("diapositivas") or []
    if not slides:
        errs.append("sin diapositivas")
    for i, s in enumerate(slides, 1):
        t = s.get("tipo")
        if t not in LAYOUTS:
            errs.append("diapositiva %d: tipo '%s' (usa %s)" % (i, t, ", ".join(LAYOUTS)))
        if t == "vinetas" and len(s.get("vinetas", [])) > 6:
            errs.append("diapositiva %d: más de 6 viñetas (STD-UI §8)" % i)
        if t == "tabla" and (not s.get("cabecera") or not s.get("filas")):
            errs.append("diapositiva %d: tabla sin cabecera o filas" % i)
        if t == "imagen" and not s.get("imagen"):
            errs.append("diapositiva %d: falta 'imagen'" % i)
    return errs


class Deck(object):
    def __init__(self, spec, root):
        from pptx import Presentation
        from pptx.util import Inches
        self.spec, self.root = spec, root
        tk = c.tokens()
        self.col, self.size = tk["color"], tk["size"]
        self.sans = tk["font"]["sansFallbackOffice"]
        self.mono = tk["font"]["monoFallbackOffice"]
        self.prs = Presentation()
        self.prs.slide_width, self.prs.slide_height = Inches(W_IN), Inches(H_IN)
        self.blank = self.prs.slide_layouts[6]
        self.n = 0
        self.cache = root / "build" / "export-cache"
        self.cache.mkdir(parents=True, exist_ok=True)

    # ------------------------------------------------------------ primitivas
    def rgb(self, key):
        from pptx.dml.color import RGBColor
        h = self.col[key].lstrip("#")
        return RGBColor(int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16))

    def text(self, slide, x, y, w, h, txt, size, color="text", bold=False, mono=False,
             align=None, anchor_bottom=False):
        from pptx.enum.text import MSO_ANCHOR, PP_ALIGN
        from pptx.util import Inches, Pt
        tb = slide.shapes.add_textbox(Inches(x), Inches(y), Inches(w), Inches(h))
        tf = tb.text_frame
        tf.word_wrap = True
        tf.margin_left = tf.margin_right = Inches(0)
        tf.vertical_anchor = MSO_ANCHOR.BOTTOM if anchor_bottom else MSO_ANCHOR.TOP
        lines = txt if isinstance(txt, list) else [txt]
        for i, line in enumerate(lines):
            p = tf.paragraphs[0] if i == 0 else tf.add_paragraph()
            r = p.add_run()
            r.text = str(line)
            r.font.size = Pt(size)
            r.font.bold = bold
            r.font.name = self.mono if mono else self.sans
            r.font.color.rgb = self.rgb(color)
            if align == "right":
                p.alignment = PP_ALIGN.RIGHT
            elif align == "center":
                p.alignment = PP_ALIGN.CENTER
        return tb

    def rect(self, slide, x, y, w, h, fill, line=None):
        from pptx.enum.shapes import MSO_SHAPE
        from pptx.util import Inches, Pt
        s = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(x), Inches(y), Inches(w), Inches(h))
        s.fill.solid()
        s.fill.fore_color.rgb = self.rgb(fill)
        if line:
            s.line.color.rgb = self.rgb(line)
            s.line.width = Pt(0.75)
        else:
            s.line.fill.background()
        s.shadow.inherit = False
        return s

    def bullets(self, slide, x, y, w, h, items, size=None):
        from pptx.util import Inches, Pt
        tb = slide.shapes.add_textbox(Inches(x), Inches(y), Inches(w), Inches(h))
        tf = tb.text_frame
        tf.word_wrap = True
        for i, it in enumerate(items):
            p = tf.paragraphs[0] if i == 0 else tf.add_paragraph()
            p.space_after = Pt(10)
            dot = p.add_run()
            dot.text = "▪  "
            dot.font.size = Pt(size or self.size["slideBodyPt"])
            dot.font.color.rgb = self.rgb("accent")
            r = p.add_run()
            r.text = str(it)
            r.font.size = Pt(size or self.size["slideBodyPt"])
            r.font.name = self.sans
            r.font.color.rgb = self.rgb("text")

    def new(self, title=None, notes=None, chrome=True):
        s = self.prs.slides.add_slide(self.blank)
        self.n += 1
        if title:
            self.text(s, 0.7, 0.45, W_IN - 1.4, 0.9, title, self.size["slideTitlePt"], bold=True,
                      anchor_bottom=True)
            self.rect(s, 0.7, 1.42, 1.2, 0.055, "accent")
        if chrome:
            self.rect(s, 0.7, H_IN - 0.55, W_IN - 1.4, 0.01, "border")
            self.text(s, 0.7, H_IN - 0.5, 8, 0.3, self.spec.get("pie") or
                      c.load_config(self.root)["project"]["name"], 10, "textMuted")
            self.text(s, W_IN - 1.7, H_IN - 0.5, 1.0, 0.3, str(self.n), 10, "textMuted",
                      align="right")
        if notes:
            s.notes_slide.notes_text_frame.text = notes
        return s

    def image_path(self, ref):
        from . import diagram
        p = Path(ref)
        p = p if p.is_absolute() else self.root / p
        if p.suffix in (".mmd", ".svg"):
            mmd = p.with_suffix(".mmd")
            png, err = diagram.png_for(mmd, self.cache)
            if not png:
                c.log("WARN", "diagrama no renderizado", file=ref, detail=err[-200:])
            return png
        return p if p.is_file() else None

    # ------------------------------------------------------------ tipos
    def s_titulo(self, d):
        s = self.new(chrome=False, notes=d.get("notas"))
        self.rect(s, 0, 0, 0.18, H_IN, "accent")
        self.text(s, 1.0, 2.2, 11, 1.4, d.get("titulo") or self.spec.get("titulo", ""), 40, bold=True,
                  anchor_bottom=True)
        self.text(s, 1.0, 3.75, 11, 0.6, d.get("subtitulo") or self.spec.get("subtitulo", ""), 20,
                  "textMuted")
        meta = " · ".join(x for x in (self.spec.get("autor"), self.spec.get("cliente"),
                                      self.spec.get("fecha")) if x)
        self.text(s, 1.0, 5.9, 11, 0.4, meta, 14, "textMuted")

    def s_seccion(self, d):
        s = self.new(notes=d.get("notas"))
        self.text(s, 0.7, 2.4, 3, 1.2, d.get("numero", ""), 54, "accentSoft", bold=True, mono=True)
        self.text(s, 0.7, 3.6, 11, 1.0, d.get("titulo", ""), 34, bold=True)
        if d.get("texto"):
            self.text(s, 0.7, 4.6, 11, 1.0, d["texto"], 16, "textMuted")

    def s_vinetas(self, d):
        s = self.new(d.get("titulo"), d.get("notas"))
        self.bullets(s, 0.7, 1.8, W_IN - 1.4, 4.9, d.get("vinetas", []))

    def s_dos_columnas(self, d):
        s = self.new(d.get("titulo"), d.get("notas"))
        half = (W_IN - 1.4 - 0.6) / 2
        for k, x in (("izquierda", 0.7), ("derecha", 0.7 + half + 0.6)):
            if d.get(k + "_titulo"):
                self.text(s, x, 1.8, half, 0.4, d[k + "_titulo"], 16, "accent", bold=True)
            self.bullets(s, x, 2.3 if d.get(k + "_titulo") else 1.8, half, 4.4, d.get(k, []), 15)

    def s_imagen(self, d):
        from pptx.util import Inches
        s = self.new(d.get("titulo"), d.get("notas"))
        png = self.image_path(d["imagen"])
        box_w, box_h, top = W_IN - 1.4, 4.6, 1.75
        if png:
            from PIL import Image
            with Image.open(str(png)) as im:
                iw, ih = im.size
            ratio = min(box_w / iw, box_h / ih)
            w, h = iw * ratio, ih * ratio
            s.shapes.add_picture(str(png), Inches(0.7 + (box_w - w) / 2), Inches(top), Inches(w),
                                 Inches(h))
        else:
            self.text(s, 0.7, 3.5, box_w, 0.5, "[imagen no disponible: %s]" % d["imagen"], 14,
                      "error")
        if d.get("pie"):
            self.text(s, 0.7, top + box_h + 0.1, box_w, 0.4, d["pie"], 12, "textMuted",
                      align="center")

    def s_tabla(self, d):
        from pptx.util import Inches, Pt
        s = self.new(d.get("titulo"), d.get("notas"))
        rows, cols = len(d["filas"]) + 1, len(d["cabecera"])
        h = min(4.8, 0.42 * rows)
        gt = s.shapes.add_table(rows, cols, Inches(0.7), Inches(1.85), Inches(W_IN - 1.4), Inches(h))
        tbl = gt.table
        tbl.first_row = True
        tbl.horz_banding = False
        for j, head in enumerate(d["cabecera"]):
            cell = tbl.cell(0, j)
            cell.fill.solid()
            cell.fill.fore_color.rgb = self.rgb("accent")
            self._cell(cell, head, "background", True)
        for i, row in enumerate(d["filas"], 1):
            for j, val in enumerate(row):
                cell = tbl.cell(i, j)
                cell.fill.solid()
                cell.fill.fore_color.rgb = self.rgb("surface" if i % 2 == 0 else "background")
                self._cell(cell, val, "text", False)
        _ = Pt

    def _cell(self, cell, val, color, bold):
        from pptx.util import Pt
        tf = cell.text_frame
        tf.text = ""
        r = tf.paragraphs[0].add_run()
        r.text = str(val)
        r.font.size = Pt(self.size["slideSmallPt"] + 1)
        r.font.name = self.sans
        r.font.bold = bold
        r.font.color.rgb = self.rgb(color)

    def s_kpis(self, d):
        s = self.new(d.get("titulo"), d.get("notas"))
        k = d.get("kpis", [])[:4]
        if not k:
            return
        gap = 0.4
        w = (W_IN - 1.4 - gap * (len(k) - 1)) / len(k)
        for i, kp in enumerate(k):
            x = 0.7 + i * (w + gap)
            self.rect(s, x, 2.2, w, 2.6, "surface")
            self.rect(s, x, 2.2, 0.06, 2.6, "accent")
            self.text(s, x + 0.35, 2.55, w - 0.5, 1.1, kp.get("valor", ""), 36, bold=True, mono=True)
            self.text(s, x + 0.35, 3.75, w - 0.5, 0.9, kp.get("etiqueta", ""), 14, "textMuted")

    def s_cierre(self, d):
        s = self.new(chrome=False, notes=d.get("notas"))
        self.rect(s, 0, 0, 0.18, H_IN, "accent")
        self.text(s, 1.0, 2.6, 11, 1.2, d.get("titulo", "Gracias"), 40, bold=True,
                  anchor_bottom=True)
        if d.get("texto"):
            self.text(s, 1.0, 3.95, 11, 1.4, d["texto"], 18, "textMuted")

    def build(self, out):
        for d in self.spec["diapositivas"]:
            getattr(self, "s_" + d["tipo"])(d)
        self.prs.core_properties.title = self.spec.get("titulo", "")
        self.prs.core_properties.author = self.spec.get("autor", "")
        self.prs.save(str(out))


def build(spec_path=None, out=None):
    root = c.project_root()
    try:
        import pptx  # noqa: F401
    except ImportError:
        c.die("falta python-pptx (pip3 install --user python-pptx); ver h doctor", c.EXIT_DEP)
    spec_path = Path(spec_path) if spec_path else root / "docs" / "04-oferta" / "presentacion.json"
    spec = c.load_json(spec_path)
    if not spec:
        c.die("no existe %s" % spec_path)
    errs = validate(spec)
    if errs:
        for e in errs:
            c.log("ERROR", e)
        return c.EXIT_FAIL
    out = Path(out) if out else root / "docs" / "export" / "oferta-presentacion.pptx"
    out.parent.mkdir(parents=True, exist_ok=True)
    Deck(spec, root).build(out)
    c.info("presentación generada: %s" % (out.relative_to(root) if root in out.parents else out))
    return c.EXIT_OK

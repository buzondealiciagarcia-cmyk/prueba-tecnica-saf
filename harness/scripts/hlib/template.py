"""Plantillas Word: plantilla base con la paleta, inspección y registro de la plantilla X."""
import json
import shutil
from pathlib import Path

from . import common as c
from . import docxutil as dx

REQ_FIELDS = ["ID", "TITULO", "DESCRIPCION", "TIPO", "PRIORIDAD", "ORIGEN", "VERIFICACION",
              "CRITERIO", "ESTADO", "SECCION"]
COVER_FIELDS = ["TITULO", "SUBTITULO", "CODIGO", "VERSION", "FECHA", "AUTOR", "CLIENTE",
                "PROYECTO"]


def _need_docx():
    if not dx.HAVE_DOCX:
        c.die("falta python-docx (pip3 install --user python-docx); ver h doctor", c.EXIT_DEP)


# --------------------------------------------------------------------------- plantilla base
def _style(doc, name, kind=1, base=None):
    from docx.enum.style import WD_STYLE_TYPE
    st = doc.styles
    try:
        return st[name]
    except KeyError:
        s = st.add_style(name, WD_STYLE_TYPE.PARAGRAPH if kind == 1 else WD_STYLE_TYPE.CHARACTER)
        if base:
            s.base_style = st[base]
        return s


def _font(style, size=None, color=None, bold=None, italic=None, name=None):
    f = style.font
    if name:
        f.name = name
        rpr = style.element.get_or_add_rPr()
        rfonts = rpr.find(dx.qn("w:rFonts"))
        if rfonts is None:
            rfonts = dx.el("w:rFonts")
            rpr.append(rfonts)
        for a in ("w:ascii", "w:hAnsi", "w:cs", "w:eastAsia"):
            rfonts.set(dx.qn(a), name)
    if size:
        f.size = dx.Pt(size)
    if color:
        f.color.rgb = dx.hex_rgb(color)
    if bold is not None:
        f.bold = bold
    if italic is not None:
        f.italic = italic


def _table_style_xml(col):
    border = col["border"].lstrip("#")
    xml = (
        '<w:style xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
        'w:type="table" w:styleId="Table"><w:name w:val="Table"/><w:basedOn w:val="TableNormal"/>'
        '<w:uiPriority w:val="59"/><w:qFormat/>'
        '<w:pPr><w:spacing w:before="40" w:after="40" w:line="240" w:lineRule="auto"/></w:pPr>'
        '<w:rPr><w:sz w:val="18"/><w:szCs w:val="18"/></w:rPr>'
        '<w:tblPr><w:tblStyleRowBandSize w:val="1"/><w:tblBorders>'
        + "".join('<w:%s w:val="single" w:sz="4" w:space="0" w:color="%s"/>' % (b, border)
                  for b in ("top", "left", "bottom", "right", "insideH", "insideV"))
        + '</w:tblBorders><w:tblCellMar><w:top w:w="40" w:type="dxa"/><w:left w:w="100" w:type="dxa"/>'
        '<w:bottom w:w="40" w:type="dxa"/><w:right w:w="100" w:type="dxa"/></w:tblCellMar></w:tblPr>'
        '<w:tblStylePr w:type="firstRow"><w:rPr><w:b/><w:color w:val="FFFFFF"/></w:rPr>'
        '<w:tcPr><w:shd w:val="clear" w:color="auto" w:fill="%s"/></w:tcPr></w:tblStylePr>'
        '<w:tblStylePr w:type="band2Horz"><w:tcPr><w:shd w:val="clear" w:color="auto" w:fill="%s"/>'
        '</w:tcPr></w:tblStylePr></w:style>'
    ) % (col["accent"].lstrip("#"), col["surface"].lstrip("#"))
    from docx.oxml import parse_xml
    return parse_xml(xml)


def _field_run(paragraph, instr):
    r = dx.el("w:r")
    r.append(dx.el("w:fldChar", **{"w:fldCharType": "begin"}))
    paragraph._p.append(r)
    r2 = dx.el("w:r")
    it = dx.el("w:instrText", **{"xml:space": "preserve"})
    it.text = " %s " % instr
    r2.append(it)
    paragraph._p.append(r2)
    r3 = dx.el("w:r")
    r3.append(dx.el("w:fldChar", **{"w:fldCharType": "separate"}))
    paragraph._p.append(r3)
    r4 = dx.el("w:r")
    t = dx.el("w:t")
    t.text = "1"
    r4.append(t)
    paragraph._p.append(r4)
    r5 = dx.el("w:r")
    r5.append(dx.el("w:fldChar", **{"w:fldCharType": "end"}))
    paragraph._p.append(r5)


def build_base(out_path):
    """Genera plantilla-base.docx con la paleta STD-UI y su mapa."""
    _need_docx()
    from docx import Document
    from docx.enum.text import WD_ALIGN_PARAGRAPH
    from docx.shared import Cm
    tk = c.tokens()
    col, fnt, sz = tk["color"], tk["font"], tk["size"]
    sans, mono = fnt["sansFallbackOffice"], fnt["monoFallbackOffice"]
    doc = Document()
    sec = doc.sections[0]
    sec.page_width, sec.page_height = Cm(21.0), Cm(29.7)
    for side in ("left_margin", "right_margin"):
        setattr(sec, side, Cm(2.3))
    sec.top_margin, sec.bottom_margin = Cm(2.5), Cm(2.2)

    normal = doc.styles["Normal"]
    _font(normal, sz["docBodyPt"], col["text"], name=sans)
    normal.paragraph_format.space_after = dx.Pt(6)
    normal.paragraph_format.line_spacing = 1.15
    for name in ("Body Text", "First Paragraph", "Compact", "Block Text"):
        s = _style(doc, name, base="Normal")
        _font(s, sz["docBodyPt"], col["text"], name=sans)
    doc.styles["Compact"].paragraph_format.space_after = dx.Pt(2)
    _font(_style(doc, "Title"), 26, col["text"], bold=True, name=sans)
    _font(_style(doc, "Subtitle"), 14, col["textMuted"], bold=False, italic=False, name=sans)
    for name in ("Author", "Date", "Abstract"):
        _font(_style(doc, name, base="Normal"), 11, col["textMuted"], name=sans)
    heads = [("Heading 1", sz["docH1Pt"], col["accent"]), ("Heading 2", sz["docH2Pt"], col["text"]),
             ("Heading 3", sz["docH3Pt"], col["text"]), ("Heading 4", 11, col["textMuted"])]
    for name, size, color in heads:
        s = doc.styles[name]
        _font(s, size, color, bold=True, italic=False, name=sans)
        s.paragraph_format.space_before = dx.Pt(14 if name == "Heading 1" else 10)
        s.paragraph_format.space_after = dx.Pt(6)
    for name in ("Caption", "Image Caption", "Table Caption"):
        s = _style(doc, name, base="Normal")
        _font(s, sz["docCaptionPt"], col["textMuted"], italic=True, name=sans)
    s = _style(doc, "Source Code", base="Normal")
    _font(s, 9, col["text"], name=mono)
    s.paragraph_format.space_after = dx.Pt(0)
    _font(_style(doc, "Verbatim Char", kind=2), 9, col["text"], name=mono)
    _font(_style(doc, "TOC Heading", base="Heading 1"), sz["docH1Pt"], col["accent"], bold=True,
          name=sans)
    hl = _style(doc, "Hyperlink", kind=2)
    _font(hl, None, col["accent"])
    doc.styles.element.append(_table_style_xml(col))

    # cabecera y pie
    hp = sec.header.paragraphs[0]
    hp.text = "{{PROYECTO}}\t\t{{CODIGO}} · v{{VERSION}}"
    for r in hp.runs:
        r.font.size, r.font.color.rgb, r.font.name = dx.Pt(8), dx.hex_rgb(col["textMuted"]), sans
    fp = sec.footer.paragraphs[0]
    fp.alignment = WD_ALIGN_PARAGRAPH.RIGHT
    run = fp.add_run("Página ")
    run.font.size = dx.Pt(8)
    _field_run(fp, "PAGE")
    run = fp.add_run(" de ")
    run.font.size = dx.Pt(8)
    _field_run(fp, "NUMPAGES")

    # cuerpo: portada (se copia al exportar), tabla de requisito y tabla de referencia
    for _ in range(6):
        doc.add_paragraph("")
    doc.add_paragraph("{{TITULO}}", style="Title")
    doc.add_paragraph("{{SUBTITULO}}", style="Subtitle")
    doc.add_paragraph("")
    doc.add_paragraph("Código {{CODIGO}} · Versión {{VERSION}} · {{FECHA}}", style="Author")
    doc.add_paragraph("{{AUTOR}}", style="Author")
    doc.add_paragraph("{{CLIENTE}}", style="Author")
    doc.add_paragraph("").add_run().add_break(dx.WD_BREAK.PAGE)

    doc.add_paragraph("Formato de requisito", style="Heading 2")
    t = doc.add_table(rows=8, cols=2)
    t.style = doc.styles["Table"]
    rows = [("{{ID}}", "{{TITULO}}"), ("Descripción", "{{DESCRIPCION}}"), ("Tipo", "{{TIPO}}"),
            ("Prioridad", "{{PRIORIDAD}}"), ("Origen", "{{ORIGEN}}"),
            ("Método de verificación", "{{VERIFICACION}}"),
            ("Criterio de aceptación", "{{CRITERIO}}"), ("Estado", "{{ESTADO}}")]
    for i, (a, b) in enumerate(rows):
        t.cell(i, 0).text, t.cell(i, 1).text = a, b
        t.cell(i, 0).width, t.cell(i, 1).width = Cm(4.5), Cm(11.9)
        if i > 0:
            dx.set_cell_shading(t.cell(i, 0), col["surface"])
            for p in t.cell(i, 0).paragraphs:
                for r in p.runs:
                    r.font.bold = True
    t.autofit = False
    t.columns[0].width, t.columns[1].width = Cm(4.5), Cm(11.9)
    doc.add_paragraph("")
    doc.add_paragraph("Tabla de referencia", style="Heading 2")
    t2 = doc.add_table(rows=3, cols=3)
    t2.style = doc.styles["Table"]
    for j, h in enumerate(("Columna", "Columna", "Valor")):
        t2.cell(0, j).text = h
    for i in (1, 2):
        for j in range(3):
            t2.cell(i, j).text = "dato"
    for tb in (t, t2):
        for cell in tb.rows[0].cells:
            dx.set_cell_shading(cell, col["accent"])
            for p in cell.paragraphs:
                for r in p.runs:
                    r.font.bold = True
                    r.font.color.rgb = dx.hex_rgb("#FFFFFF")
    out_path = Path(out_path)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    doc.save(str(out_path))
    mapping = {
        "$comment": "Mapa de la plantilla base. Ver skill export-word para el formato.",
        "cover": {"mode": "template"},
        "requirement_table": {"table_index": 0, "mode": "per-requirement", "fill": "placeholders"},
        "table_reference": {"table_index": 1, "header_rows": 1},
        "decisions": [],
    }
    c.save_json(out_path.with_suffix(".map.json"), mapping)
    c.info("plantilla base generada: %s" % out_path)
    return c.EXIT_OK


# --------------------------------------------------------------------------- inspección
def inspect(path, as_json=False):
    """Describe estilos, secciones, cabecera/pie, párrafos y tablas de una plantilla."""
    _need_docx()
    from docx import Document
    doc = Document(str(path))
    info = {"file": str(path), "sections": [], "body": [], "tables": [], "placeholders": [],
            "paragraph_styles_used": [], "table_styles": []}
    for s in doc.sections:
        info["sections"].append({
            "page_cm": [round(s.page_width.cm, 2), round(s.page_height.cm, 2)],
            "margins_cm": [round(x.cm, 2) for x in (s.top_margin, s.right_margin,
                                                     s.bottom_margin, s.left_margin)],
            "header": [p.text for p in s.header.paragraphs if p.text.strip()],
            "footer": [p.text for p in s.footer.paragraphs if p.text.strip()],
        })
    used = set()
    ti = 0
    from docx.table import Table
    from docx.text.paragraph import Paragraph
    for e in dx.body_elements(doc):
        if e.tag == dx.qn("w:p"):
            p = Paragraph(e, doc)
            used.add(p.style.name)
            info["body"].append({"type": "p", "style": p.style.name, "text": p.text[:100],
                                 "page_break": dx.paragraph_has_page_break(e)})
        elif e.tag == dx.qn("w:tbl"):
            t = Table(e, doc)
            cells = [[dx.cell_text(cl)[:60] for cl in r.cells] for r in t.rows]
            info["tables"].append({"index": ti, "style": t.style.name if t.style else None,
                                   "rows": len(t.rows), "cols": len(t.columns), "cells": cells})
            info["body"].append({"type": "table", "index": ti})
            ti += 1
    for p in dx.iter_paragraphs_deep(doc):
        for m in dx.PH_RE.finditer(p.text):
            info["placeholders"].append(m.group(1))
    info["paragraph_styles_used"] = sorted(used)
    info["table_styles"] = sorted(s.name for s in doc.styles if s.type == 3)
    if as_json:
        print(json.dumps(info, indent=2, ensure_ascii=False))
        return c.EXIT_OK
    out = ["# Inspección de plantilla: %s" % Path(path).name, ""]
    for i, s in enumerate(info["sections"]):
        out.append("- Sección %d: página %s cm, márgenes %s cm" % (i, s["page_cm"], s["margins_cm"]))
        out.append("  - cabecera: %s" % (s["header"] or "—"))
        out.append("  - pie: %s" % (s["footer"] or "—"))
    out.append("\n## Cuerpo (orden)\n")
    for b in info["body"]:
        if b["type"] == "p":
            if b["text"].strip() or b["page_break"]:
                out.append("- ¶ [%s] %s%s" % (b["style"], b["text"],
                                              "  ⟵ salto de página" if b["page_break"] else ""))
        else:
            t = info["tables"][b["index"]]
            out.append("- TABLA %d (%d×%d, estilo %s)" % (t["index"], t["rows"], t["cols"],
                                                         t["style"]))
            for r_i, row in enumerate(t["cells"]):
                out.append("    - fila %d: %s" % (r_i, " | ".join(row)))
    out.append("\nMarcadores {{...}} encontrados: %s" % (sorted(set(info["placeholders"])) or "ninguno"))
    out.append("Estilos de tabla disponibles: %s" % ", ".join(info["table_styles"]))
    print("\n".join(out))
    return c.EXIT_OK


def register(src, name=None):
    """Copia la plantilla X al proyecto y prepara un mapa inicial para revisar con el usuario."""
    _need_docx()
    root = c.project_root()
    src = Path(src).expanduser().resolve()
    if not src.is_file():
        c.die("no existe %s" % src, c.EXIT_USAGE)
    dst_dir = root / "docs" / "plantillas"
    dst_dir.mkdir(parents=True, exist_ok=True)
    dst = dst_dir / (name or src.name)
    if src.suffix.lower() == ".dotx":
        dst = dst.with_suffix(".docx")
        _dotx_to_docx(src, dst)
    else:
        shutil.copyfile(str(src), str(dst))
    map_path = root / "config" / "template-map.json"
    if not map_path.is_file():
        c.save_json(map_path, {
            "$comment": "Revisar con el usuario: qué tabla es el formato de requisito, cómo se "
                        "rellena (placeholders o coordenadas) y qué tabla es la de referencia.",
            "cover": {"mode": "pandoc"},
            "requirement_table": {"table_index": 0, "mode": "per-requirement",
                                  "fill": "placeholders", "fields": {}},
            "table_reference": {"table_index": 0, "header_rows": 1},
            "decisions": [],
        })
    cfg_path = root / c.CONFIG_NAME
    cfg = c.load_json(cfg_path, {})
    cfg.setdefault("export", {})
    cfg["export"]["template"] = str(dst.relative_to(root))
    cfg["export"]["template_map"] = "config/template-map.json"
    c.save_json(cfg_path, cfg)
    c.info("plantilla registrada: %s\nmapa a completar: %s" % (dst.relative_to(root),
                                                              map_path.relative_to(root)))
    return inspect(dst)


def _dotx_to_docx(src, dst):
    import zipfile
    with zipfile.ZipFile(str(src)) as zin, zipfile.ZipFile(str(dst), "w",
                                                           zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "[Content_Types].xml":
                data = data.replace(
                    b"application/vnd.openxmlformats-officedocument.wordprocessingml.template.main+xml",
                    b"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml")
            zout.writestr(item, data)


def validate_map(mapping, template_path):
    _need_docx()
    from docx import Document
    errs = []
    doc = Document(str(template_path))
    n = len(doc.tables)
    for key in ("requirement_table", "table_reference"):
        sect = mapping.get(key)
        if sect and sect.get("table_index") is not None and sect["table_index"] >= n:
            errs.append("%s.table_index=%s pero la plantilla tiene %d tablas" % (
                key, sect["table_index"], n))
    rt = mapping.get("requirement_table") or {}
    if rt.get("fill") == "cells":
        for f, rc in (rt.get("fields") or {}).items():
            if f not in REQ_FIELDS:
                errs.append("campo desconocido %s (usa %s)" % (f, ", ".join(REQ_FIELDS)))
            if not (isinstance(rc, list) and len(rc) == 2):
                errs.append("fields.%s debe ser [fila, columna]" % f)
    if rt.get("mode") == "rows" and not rt.get("columns"):
        errs.append("mode=rows requiere 'columns': {CAMPO: índice}")
    return errs

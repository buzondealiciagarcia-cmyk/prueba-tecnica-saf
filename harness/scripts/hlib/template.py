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
        for a in ("w:asciiTheme", "w:hAnsiTheme", "w:cstheme", "w:eastAsiaTheme"):
            if rfonts.get(dx.qn(a)) is not None:
                del rfonts.attrib[dx.qn(a)]
    if size:
        f.size = dx.Pt(size)
    if color:
        f.color.rgb = dx.hex_rgb(color)
    if bold is not None:
        f.bold = bold
    if italic is not None:
        f.italic = italic


def _table_style_xml(dk):
    """Estilo de tabla 'Table': cabecera gris, bordes finos negros (plantilla del usuario)."""
    border = dk["tableBorder"].lstrip("#")
    sz = dk["tableBorderSz"]
    xml = (
        '<w:style xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
        'w:type="table" w:styleId="Table"><w:name w:val="Table"/><w:basedOn w:val="TableNormal"/>'
        '<w:uiPriority w:val="59"/><w:qFormat/>'
        '<w:pPr><w:spacing w:before="30" w:after="30" w:line="240" w:lineRule="auto"/></w:pPr>'
        '<w:rPr><w:sz w:val="%d"/><w:szCs w:val="%d"/></w:rPr>'
        '<w:tblPr><w:tblBorders>'
        + "".join('<w:%s w:val="single" w:sz="%d" w:space="0" w:color="%s"/>' % (b, sz, border)
                  for b in ("top", "left", "bottom", "right", "insideH", "insideV"))
        + '</w:tblBorders><w:tblCellMar><w:top w:w="30" w:type="dxa"/><w:left w:w="80" w:type="dxa"/>'
        '<w:bottom w:w="30" w:type="dxa"/><w:right w:w="80" w:type="dxa"/></w:tblCellMar></w:tblPr>'
        '<w:tblStylePr w:type="firstRow"><w:rPr><w:b/></w:rPr>'
        '<w:tcPr><w:shd w:val="clear" w:color="auto" w:fill="%s"/></w:tcPr></w:tblStylePr>'
        '</w:style>'
    ) % (int(dk["tablePt"] * 2), int(dk["tablePt"] * 2), dk["tableHeader"].lstrip("#"))
    from docx.oxml import parse_xml
    return parse_xml(xml)


def _field_run(paragraph, instr, size=None):
    for kind in ("begin", None, "separate", "text", "end"):
        r = dx.el("w:r")
        if size:
            rpr = dx.el("w:rPr")
            rpr.append(dx.el("w:sz", **{"w:val": str(int(size * 2))}))
            r.append(rpr)
        if kind in ("begin", "separate", "end"):
            r.append(dx.el("w:fldChar", **{"w:fldCharType": kind}))
        elif kind is None:
            it = dx.el("w:instrText", **{"xml:space": "preserve"})
            it.text = " %s " % instr
            r.append(it)
        else:
            t = dx.el("w:t")
            t.text = "1"
            r.append(t)
        paragraph._p.append(r)


def _styles(doc, dk):
    """Estilos de párrafo comunes (Arial, negro) generalizados desde la plantilla."""
    sans, mono = dk["font"], dk["fontMono"]
    txt, muted = dk["text"], dk["textMuted"]
    normal = doc.styles["Normal"]
    _font(normal, dk["bodyPt"], txt, name=sans)
    normal.paragraph_format.space_after = dx.Pt(6)
    normal.paragraph_format.line_spacing = 1.1
    for name in ("Body Text", "First Paragraph", "Compact", "Block Text"):
        _font(_style(doc, name, base="Normal"), dk["bodyPt"], txt, name=sans)
    doc.styles["Compact"].paragraph_format.space_after = dx.Pt(2)
    _font(_style(doc, "Title"), dk["titlePt"], txt, bold=True, name=sans)
    ttl = doc.styles["Title"]
    ttl.paragraph_format.space_after = dx.Pt(8)
    pPr = ttl.element.get_or_add_pPr()
    for b in pPr.findall(dx.qn("w:pBdr")):
        pPr.remove(b)
    _font(_style(doc, "Subtitle"), dk["subtitlePt"], muted, bold=False, italic=False, name=sans)
    for name in ("Author", "Date", "Abstract"):
        _font(_style(doc, name, base="Normal"), 11, muted, name=sans)
    for name, size in (("Heading 1", dk["h1Pt"]), ("Heading 2", dk["h2Pt"]),
                       ("Heading 3", dk["h3Pt"]), ("Heading 4", dk["bodyPt"])):
        s = doc.styles[name]
        _font(s, size, txt, bold=True, italic=False, name=sans)
        s.paragraph_format.space_before = dx.Pt(12 if name == "Heading 1" else 8)
        s.paragraph_format.space_after = dx.Pt(4)
    for name in ("Caption", "Image Caption", "Table Caption"):
        _font(_style(doc, name, base="Normal"), dk["captionPt"], muted, italic=True, name=sans)
    s = _style(doc, "Source Code", base="Normal")
    _font(s, 9, txt, name=mono)
    s.paragraph_format.space_after = dx.Pt(0)
    _font(_style(doc, "Verbatim Char", kind=2), 9, txt, name=mono)
    _font(_style(doc, "TOC Heading", base="Heading 1"), dk["h1Pt"], txt, bold=True, name=sans)
    _font(_style(doc, "Hyperlink", kind=2), None, txt)
    doc.styles.element.append(_table_style_xml(dk))


def _grey_table(doc, dk, headers, widths_cm, section_row=True, data_rows=1):
    """Tabla con el formato de la plantilla: cabecera #D9D9D9, fila de sección #F2F2F2."""
    from docx.shared import Cm
    rows = 1 + (1 if section_row else 0) + data_rows
    t = doc.add_table(rows=rows, cols=len(headers))
    t.style = doc.styles["Table"]
    t.autofit = False
    for j, w in enumerate(widths_cm):
        t.columns[j].width = Cm(w)
        for r in t.rows:
            r.cells[j].width = Cm(w)
    for j, h in enumerate(headers):
        cell = t.cell(0, j)
        cell.text = h
        dx.set_cell_shading(cell, dk["tableHeader"])
        for r in cell.paragraphs[0].runs:
            r.font.bold, r.font.size = True, dx.Pt(dk["tablePt"])
    trPr = t.rows[0]._tr.get_or_add_trPr()
    trPr.append(dx.el("w:tblHeader"))
    first_data = 1
    if section_row:
        merged = t.cell(1, 0).merge(t.cell(1, len(headers) - 1))
        merged.text = "{{SECCION}}"
        dx.set_cell_shading(merged, dk["tableSection"])
        for r in merged.paragraphs[0].runs:
            r.font.bold, r.font.size = True, dx.Pt(dk["tablePt"])
        first_data = 2
    for i in range(first_data, rows):
        for j in range(len(headers)):
            run = t.cell(i, j).paragraphs[0].add_run("")
            run.font.size = dx.Pt(dk["tablePt"])
    return t


def _page(sec, landscape, margin_cm):
    from docx.enum.section import WD_ORIENT
    from docx.shared import Cm
    if landscape:
        sec.orientation = WD_ORIENT.LANDSCAPE
        sec.page_width, sec.page_height = Cm(29.7), Cm(21.0)
    else:
        sec.page_width, sec.page_height = Cm(21.0), Cm(29.7)
    for side in ("left_margin", "right_margin", "top_margin", "bottom_margin"):
        setattr(sec, side, Cm(margin_cm))


REQ_HEADERS = ["Code", "Description", "Type", "Source", "Priority", "Verification",
               "Acceptance criterion", "Status"]
REQ_COLUMNS = {"ID": 0, "DESCRIPCION": 1, "TIPO": 2, "ORIGEN": 3, "PRIORIDAD": 4,
               "VERIFICACION": 5, "CRITERIO": 6, "ESTADO": 7}


def build_base(out_path):
    """Genera las dos plantillas Word del proyecto a partir del estilo del usuario.

    - plantilla-requisitos.docx: A4 apaisado; título + convenciones + tabla única
      Code|Description|Type|Source + Priority|Verification|Acceptance criterion|Status
      con filas de sección (decisión del usuario: columnas extra en el Word).
    - plantilla-documentos.docx: A4 vertical, mismo estilo, portada simple, índice y pie
      con numeración (generalización acordada para el resto de documentos).
    """
    _need_docx()
    from docx import Document
    from docx.enum.text import WD_ALIGN_PARAGRAPH
    dk = c.tokens()["docx"]
    out_dir = Path(out_path).parent if str(out_path).endswith(".docx") else Path(out_path)
    out_dir.mkdir(parents=True, exist_ok=True)

    # ---- requisitos
    doc = Document()
    _page(doc.sections[0], True, 2.0)
    _styles(doc, dk)
    doc.add_paragraph("{{TITULO}}", style="Title")
    doc.add_paragraph("Párrafos de convenciones (se toman del markdown).")
    _grey_table(doc, dk, REQ_HEADERS, [2.4, 8.6, 2.2, 2.1, 1.7, 2.3, 4.6, 1.8])
    req_path = out_dir / "plantilla-requisitos.docx"
    doc.save(str(req_path))
    c.save_json(req_path.with_suffix(".map.json"), {
        "$comment": "Formato de requisitos del usuario (Requirements_2.pages) con 4 columnas extra.",
        "document": {"cover": "title-only", "toc": False, "control_table": False,
                     "header_footer": False, "strip_sections": ["Historial de cambios"]},
        "requirement_table": {"table_index": 0, "mode": "rows", "header_rows": 1,
                              "section_row": 1, "data_row": 2, "columns": REQ_COLUMNS},
        "table_reference": {"table_index": 0, "header_rows": 1, "body_rows": [2]},
        "decisions": [
            "2026-11-01 · Plantilla original en .pages: se recrea en .docx (Arial, negro, cabecera "
            "#D9D9D9, filas de sección #F2F2F2, bordes negros finos).",
            "2026-11-01 · Prioridad, verificación, criterio de aceptación y estado se exportan como "
            "columnas extra (Priority, Verification, Acceptance criterion, Status); por ello el "
            "documento de requisitos es A4 apaisado.",
            "2026-11-01 · El documento de requisitos no lleva portada, índice ni tabla de control "
            "(idéntico a la plantilla).",
        ],
    })

    # ---- resto de documentos
    doc = Document()
    sec = doc.sections[0]
    _page(sec, False, 2.5)
    _styles(doc, dk)
    fp = sec.footer.paragraphs[0]
    fp.alignment = WD_ALIGN_PARAGRAPH.RIGHT
    for txt, fld in (("{{CODIGO}} · v{{VERSION}} · Página ", "PAGE"), (" de ", "NUMPAGES")):
        r = fp.add_run(txt)
        r.font.size, r.font.name = dx.Pt(8), dk["font"]
        r.font.color.rgb = dx.hex_rgb(dk["textMuted"])
        _field_run(fp, fld, 8)
    doc.add_paragraph("{{TITULO}}", style="Title")
    _grey_table(doc, dk, ["Columna", "Columna", "Valor"], [5.3, 5.3, 5.4], section_row=True)
    # tema del documento: fuentes mayor/menor también en Arial para estilos heredados

    doc_path = out_dir / "plantilla-documentos.docx"
    doc.save(str(doc_path))
    c.save_json(doc_path.with_suffix(".map.json"), {
        "$comment": "Generalización del estilo de la plantilla de requisitos al resto de documentos.",
        "document": {"cover": "pandoc", "toc": True, "control_table": True, "header_footer": True},
        "requirement_table": None,
        "table_reference": {"table_index": 0, "header_rows": 1, "body_rows": [2]},
        "decisions": [
            "2026-11-01 · Resto de documentos: mismo estilo (Arial, negro, tablas grises) con "
            "portada simple, índice y pie con código, versión y página.",
        ],
    })
    old = out_dir / "plantilla-base.docx"
    for p in (old, old.with_suffix(".map.json")):
        if p.exists():
            p.unlink()
    c.info("plantillas generadas: %s, %s" % (req_path.name, doc_path.name))
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

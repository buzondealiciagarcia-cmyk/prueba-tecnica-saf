"""Exportación markdown -> Word con la plantilla registrada (skill export-word).

Flujo: preprocesado (diagramas Mermaid a PNG, numeración de figuras y tablas,
marcadores de requisitos) -> pandoc con la plantilla como reference-doc ->
postprocesado con python-docx (portada de la plantilla, tablas con el formato de
la tabla de referencia, requisitos en el formato de la plantilla, cabecera/pie).
"""
import copy
import re
import struct
import tempfile
from pathlib import Path

from . import common as c
from . import docxutil as dx
from . import reqs as rq

REQ_PH = "⟦H-REQ:%s⟧"
REQ_PH_RE = re.compile(r"⟦H-REQ:([^⟧]+)⟧")
REQTAB_PH = "⟦H-REQTABLE:%s⟧"
REQTAB_PH_RE = re.compile(r"⟦H-REQTABLE:([^⟧]+)⟧")
LAND_PH = "⟦H-LANDSCAPE:%s⟧"
LAND_PH_RE = re.compile(r"⟦H-LANDSCAPE:(begin|end)⟧")
IMG_RE = re.compile(r"!\[([^\]]*)\]\(([^)\s]+)\)(\{[^}]*\})?")
FIG_PREFIX_RE = re.compile(r"^\s*Figura(\s+\d+)?\s*[.:]\s*", re.I)
TAB_CAPTION_RE = re.compile(r"^\s*Tabla(\s+\d+)?\s*[.:]\s*(.+)$", re.I)

KNOWN_DOCS = {
    "requisitos.md": ("Especificación de requisitos", "REQ"),
    "diseno.md": ("Documento de diseño", "DIS"),
    "comparativa.md": ("Comparativa de alternativas de diseño", "ALT"),
    "implementacion.md": ("Documento de implementación", "IMP"),
    "oferta.md": ("Oferta técnica y económica", "OFE"),
    "dosier.md": ("Dosier del proyecto", "DOS"),
}


# --------------------------------------------------------------------------- utilidades
def _png_size(path):
    with open(str(path), "rb") as fh:
        head = fh.read(24)
    if head[:8] != b"\x89PNG\r\n\x1a\n":
        return None
    return struct.unpack(">II", head[16:24])


def _text_width_cm(template):
    if not dx.HAVE_DOCX:
        return 16.0
    from docx import Document
    s = Document(str(template)).sections[0]
    return round((s.page_width - s.left_margin - s.right_margin) / 360000.0, 2)


def _control_table(text):
    """Primera tabla 'Campo | Valor' del documento -> dict normalizado."""
    for table in c.md_tables(text)[:2]:
        if table and len(table[0]) >= 2:
            meta = {}
            for row in table:
                if len(row) >= 2:
                    meta[c.norm(row[0])] = row[1]
            if any(k in meta for k in ("version", "codigo", "estado", "fecha")):
                return meta
    return {}


# --------------------------------------------------------------------------- preprocesado
class Pre(object):
    def __init__(self, src, cache, text_width_cm, req_mode, req_sections=None):
        self.src = Path(src)
        self.base = self.src.parent
        self.cache = Path(cache)
        self.width = text_width_cm
        self.req_mode = req_mode
        self.req_sections = req_sections or set()
        self.fig = 0
        self.tab = 0
        self.warnings = []

    def _img_md(self, caption, png):
        self.fig += 1
        caption = FIG_PREFIX_RE.sub("", caption or "").strip() or "Diagrama"
        attr = ""
        size = _png_size(png)
        if size:
            natural_cm = size[0] / 3.0 / 96.0 * 2.54
            attr = "{width=%.1fcm}" % min(natural_cm, self.width)
        return "![Figura %d. %s](%s)%s" % (self.fig, caption, png, attr)

    def _mermaid(self, code, caption):
        from . import diagram
        tmp = self.cache / ("inline-%d.mmd" % (self.fig + 1))
        tmp.write_text(code, encoding="utf-8")
        png, err = diagram.png_for(tmp, self.cache)
        if not png:
            self.warnings.append("diagrama en línea no renderizado: %s" % err[-200:])
            return "```\n%s\n```" % code
        return self._img_md(caption, png)

    def _image(self, m):
        from . import diagram
        caption, target = m.group(1), m.group(2)
        if re.match(r"https?://", target):
            return m.group(0)
        p = (self.base / target).resolve()
        mmd = p.with_suffix(".mmd")
        if mmd.is_file():
            png, err = diagram.png_for(mmd, self.cache)
            if png:
                return self._img_md(caption, png)
            self.warnings.append("no se pudo renderizar %s: %s" % (mmd.name, err[-200:]))
        if p.suffix.lower() == ".svg":
            self.warnings.append("SVG sin .mmd hermano (no exportable): %s" % target)
            return "*[Figura no disponible: %s]*" % caption
        if p.is_file():
            self.fig += 1
            cap = FIG_PREFIX_RE.sub("", caption).strip()
            return "![Figura %d. %s](%s)" % (self.fig, cap, p)
        self.warnings.append("imagen no encontrada: %s" % target)
        return "*[Imagen no encontrada: %s]*" % target

    def run(self, text):
        text = re.sub(r"<!--\s*h:landscape:(begin|end)\s*-->", lambda m: LAND_PH % m.group(1), text)
        text = re.sub(r"<!--.*?-->", "", text, flags=re.S)
        lines = text.split("\n")
        out = []
        i = 0
        pending_tab_caption = None
        req_mode = self.req_mode
        table_emitted = False
        while i < len(lines):
            line = lines[i]
            # requisitos en modo 'rows': las secciones con requisitos se sustituyen por una tabla
            if req_mode == "rows" and re.match(r"^##\s+", line) and not line.startswith("###"):
                name = re.sub(r"^##\s+", "", line).strip()
                if name in self.req_sections:
                    if not table_emitted:
                        out += ["", REQTAB_PH % "all", ""]
                        table_emitted = True
                    i += 1
                    while i < len(lines) and not (re.match(r"^##\s+", lines[i])
                                                  and not lines[i].startswith("###")):
                        i += 1
                    continue
            # bloque mermaid
            if line.strip().startswith("```mermaid"):
                code = []
                i += 1
                while i < len(lines) and not lines[i].strip().startswith("```"):
                    code.append(lines[i])
                    i += 1
                cap = ""
                for cl in code:
                    mm = re.match(r"\s*%%\s*caption:\s*(.+)", cl)
                    if mm:
                        cap = mm.group(1)
                out.append(self._mermaid("\n".join(code), cap))
                i += 1
                continue
            # requisitos: '### ID — título' + tabla de campos -> marcador
            if req_mode == "per-requirement" and rq.HEAD_RE.match(line):
                out.append(line)
                j = i + 1
                while j < len(lines) and not lines[j].strip():
                    j += 1
                if j < len(lines) and lines[j].strip().startswith("|"):
                    while j < len(lines) and lines[j].strip().startswith("|"):
                        j += 1
                    rid = rq.HEAD_RE.match(line).group(1)
                    out.append("")
                    out.append(REQ_PH % rid)
                    out.append("")
                    i = j
                    continue
            # leyenda de tabla
            mt = TAB_CAPTION_RE.match(line)
            if mt and i + 1 < len(lines) and (lines[i + 1].strip().startswith("|") or (
                    not lines[i + 1].strip() and i + 2 < len(lines)
                    and lines[i + 2].strip().startswith("|"))):
                pending_tab_caption = mt.group(2).strip()
                i += 1
                continue
            if pending_tab_caption and line.strip().startswith("|"):
                block = []
                while i < len(lines) and lines[i].strip().startswith("|"):
                    block.append(lines[i])
                    i += 1
                self.tab += 1
                out.extend(block)
                out.append("")
                out.append(": Tabla %d. %s" % (self.tab, pending_tab_caption))
                out.append("")
                pending_tab_caption = None
                continue
            line = IMG_RE.sub(self._image, line)
            # enlaces a otros .md -> texto
            line = re.sub(r"\[([^\]]+)\]\((?!https?://)[^)]+\.md(#[^)]*)?\)", r"\1", line)
            out.append(line)
            i += 1
        return "\n".join(out)


# --------------------------------------------------------------------------- postprocesado
def _replace_ph_doc(doc, values):
    dx.replace_placeholders_in_element(list(dx.iter_paragraphs_deep(doc)), values)
    for s in doc.sections:
        for part in (s.header, s.footer, s.first_page_header, s.first_page_footer):
            try:
                dx.replace_placeholders_in_element(list(dx.iter_paragraphs_deep(part)), values)
            except Exception:  # noqa: BLE001 - cabeceras inexistentes
                pass


def _apply_cover(doc, tpl, mapping, values):
    body = doc.element.body
    # quitar bloque de título de pandoc
    from docx.text.paragraph import Paragraph
    first_non_title = None
    for e in list(body):
        if e.tag != dx.qn("w:p"):
            first_non_title = e
            break
        st = Paragraph(e, doc).style.name
        if st in ("Title", "Subtitle", "Author", "Date", "Abstract"):
            continue
        first_non_title = e
        break
    mode = (mapping.get("document") or {}).get("cover") or \
        (mapping.get("cover") or {}).get("mode", "pandoc")
    if mode in ("title-only", "none"):
        for e in list(body):
            if e is first_non_title:
                break
            if e.tag == dx.qn("w:p"):
                st = Paragraph(e, doc).style.name
                if st in ("Subtitle", "Author", "Date", "Abstract") or \
                        (mode == "none" and st == "Title"):
                    body.remove(e)
        return
    if mode == "template":
        for e in list(body):
            if e is first_non_title:
                break
            if e.tag == dx.qn("w:p") and Paragraph(e, doc).style.name in (
                    "Title", "Subtitle", "Author", "Date", "Abstract"):
                body.remove(e)
        cover = []
        for e in dx.body_elements(tpl):
            ce = copy.deepcopy(e)
            for sp in ce.iter(dx.qn("w:sectPr")):
                sp.getparent().remove(sp)
            dx.copy_images(tpl, doc, ce)
            cover.append(ce)
            if e.tag == dx.qn("w:p") and dx.paragraph_has_page_break(e):
                break
        else:
            cover.append(dx.page_break_paragraph())
        for ce in reversed(cover):
            body.insert(0, ce)
    else:
        if first_non_title is not None:
            first_non_title.addprevious(dx.page_break_paragraph())


def _toc_page_break(doc):
    for sdt in doc.element.body.iter(dx.qn("w:sdt")):
        if "TOC" in "".join(t.text or "" for t in sdt.iter(dx.qn("w:instrText"))):
            sdt.addnext(dx.page_break_paragraph())
            break


def _fill_req_table(tbl_el, doc, mapping, req):
    from docx.table import Table
    t = Table(tbl_el, doc)
    vals = _req_values(req)
    rt = mapping.get("requirement_table") or {}
    if rt.get("fill") == "cells":
        for field, rc in (rt.get("fields") or {}).items():
            dx.set_cell_text(t.cell(rc[0], rc[1]), vals.get(field, ""))
    else:
        dx.replace_placeholders_in_element(list(dx.iter_paragraphs_deep(t)), vals)


PENDING_RE = re.compile(r"\[PENDIENTE(?::\s*([^\]]*))?\]")


def pending_to_template(text):
    """'[PENDIENTE: 50 ms]' -> '[50 ms]'; '[PENDIENTE]' -> '[X]' (convención de la plantilla)."""
    return PENDING_RE.sub(lambda m: "[%s]" % (m.group(1).strip() if m.group(1) else "X"),
                          text or "")


def _md_inline(text):
    """Quita marcas markdown simples dentro de celdas (negrita, cursiva, código)."""
    text = re.sub(r"\*\*(.+?)\*\*|__(.+?)__", lambda m: m.group(1) or m.group(2), text)
    text = re.sub(r"`([^`]+)`", r"\1", text)
    return text.replace("<br>", "\n").replace("\\|", "|")


def _req_values(r):
    f = r["campos"]
    vals = {"ID": r["id"], "TITULO": r["titulo"], "SECCION": rq.section_name(r.get("seccion", "")),
            "DESCRIPCION": f.get("descripcion", ""), "TIPO": f.get("tipo", ""),
            "PRIORIDAD": f.get("prioridad", ""), "ORIGEN": f.get("origen", ""),
            "VERIFICACION": f.get("verificacion", ""), "CRITERIO": f.get("criterio", ""),
            "ESTADO": f.get("estado", "")}
    return {k: _md_inline(pending_to_template(v)) or ("—" if k != "ID" else v)
            for k, v in vals.items()}


def _requirements_rows(doc, tpl, rt, req_list, produced, only=None):
    """Una tabla con cabecera, filas de sección y una fila por requisito (modo 'rows').

    only: si se indica, solo se sustituyen los marcadores cuyo argumento coincide.
    """
    from docx.table import Table
    from docx.text.paragraph import Paragraph
    hdr = int(rt.get("header_rows", 1))
    sec_i, data_i = rt.get("section_row"), int(rt.get("data_row", hdr))
    cols = rt.get("columns") or {}
    for p_el in list(doc.element.body.iter(dx.qn("w:p"))):
        m = REQTAB_PH_RE.search(Paragraph(p_el, doc).text)
        if not m or (only is not None and m.group(1).strip() != only):
            continue
        new_tbl = copy.deepcopy(tpl.tables[rt["table_index"]]._tbl)
        rows = new_tbl.findall(dx.qn("w:tr"))
        sec_tpl = copy.deepcopy(rows[sec_i]) if sec_i is not None else None
        data_tpl = copy.deepcopy(rows[data_i])
        for tr in rows[hdr:]:
            new_tbl.remove(tr)
        current = None
        for r in req_list:
            sec = rq.section_name(r.get("seccion", ""))
            if sec_tpl is not None and sec != current:
                tr = copy.deepcopy(sec_tpl)
                new_tbl.append(tr)
                current = sec
                t = Table(new_tbl, doc)
                dx.set_cell_text(t.rows[-1].cells[0], sec)
            tr = copy.deepcopy(data_tpl)
            new_tbl.append(tr)
            t = Table(new_tbl, doc)
            vals = _req_values(r)
            row = t.rows[-1]
            for field, ci in cols.items():
                dx.set_cell_text(row.cells[ci], vals.get(field, ""))
        p_el.addnext(new_tbl)
        produced.append(new_tbl)
        p_el.getparent().remove(p_el)


def _external_requirements(doc, root, cfg, produced):
    """Marcadores ⟦H-REQTABLE:<ruta requisitos.md>⟧ en otros documentos (p.ej. el dosier)."""
    from docx import Document
    from docx.text.paragraph import Paragraph
    paths = set()
    for p_el in doc.element.body.iter(dx.qn("w:p")):
        m = REQTAB_PH_RE.search(Paragraph(p_el, doc).text)
        if m and m.group(1).strip().endswith(".md"):
            paths.add(m.group(1).strip())
    if not paths:
        return
    rcfg = dict(cfg["export"])
    rcfg.update((cfg["export"].get("documents") or {}).get("requisitos") or {})
    rtpl = Document(str(c.expand(rcfg["template"], root)))
    rmap = c.load_json(c.expand(rcfg["template_map"], root), {})
    rt = rmap.get("requirement_table") or {}
    for rel in paths:
        src = root / rel
        req_list = rq.parse(src.read_text(encoding="utf-8")) if src.is_file() else []
        _requirements_rows(doc, rtpl, rt, req_list, produced, only=rel)


def _landscape_sections(doc):
    """Convierte ⟦H-LANDSCAPE:begin/end⟧ en saltos de sección apaisados."""
    from docx.text.paragraph import Paragraph
    body_sp = doc.element.body.find(dx.qn("w:sectPr"))
    if body_sp is None:
        return
    for p_el in list(doc.element.body.iter(dx.qn("w:p"))):
        m = LAND_PH_RE.search(Paragraph(p_el, doc).text)
        if not m:
            continue
        sp = copy.deepcopy(body_sp)
        if m.group(1) == "end":
            pg = sp.find(dx.qn("w:pgSz"))
            if pg is not None:
                w, h = pg.get(dx.qn("w:w")), pg.get(dx.qn("w:h"))
                pg.set(dx.qn("w:w"), h)
                pg.set(dx.qn("w:h"), w)
                pg.set(dx.qn("w:orient"), "landscape")
            mar = sp.find(dx.qn("w:pgMar"))
            if mar is not None:
                for k in ("w:top", "w:bottom", "w:left", "w:right"):
                    mar.set(dx.qn(k), "1134")
        new_p = dx.el("w:p")
        ppr = dx.el("w:pPr")
        ppr.append(sp)
        new_p.append(ppr)
        p_el.addnext(new_p)
        p_el.getparent().remove(p_el)


def _requirements(doc, tpl, mapping, req_list, produced):
    from docx.text.paragraph import Paragraph
    rt = mapping.get("requirement_table")
    if not rt or rt.get("table_index") is None:
        return
    if rt.get("mode") == "rows":
        return _requirements_rows(doc, tpl, rt, req_list, produced)
    src_tbl = tpl.tables[rt["table_index"]]._tbl
    by_id = {r["id"]: r for r in req_list}
    for p_el in list(doc.element.body.iter(dx.qn("w:p"))):
        txt = Paragraph(p_el, doc).text
        m = REQ_PH_RE.search(txt)
        if not m:
            continue
        r = by_id.get(m.group(1).strip())
        new_tbl = copy.deepcopy(src_tbl)
        dx.copy_images(tpl, doc, new_tbl)
        p_el.addnext(new_tbl)
        if r:
            _fill_req_table(new_tbl, doc, mapping, r)
        produced.append(new_tbl)
        spacer = dx.el("w:p")
        new_tbl.addnext(spacer)
        p_el.getparent().remove(p_el)


def _style_tables(doc, tpl, mapping, produced):
    tr = mapping.get("table_reference")
    ref = None
    if tr and tr.get("table_index") is not None and tr["table_index"] < len(tpl.tables):
        ref = tpl.tables[tr["table_index"]]
    hdr_rows = int((tr or {}).get("header_rows", 1))
    for t in doc.tables:
        if any(t._tbl is e for e in produced):
            continue
        tblPr = t._tbl.tblPr
        if ref is not None:
            new_pr = copy.deepcopy(ref._tbl.tblPr)
            for w in new_pr.findall(dx.qn("w:tblW")):
                new_pr.remove(w)
            new_pr.append(dx.el("w:tblW", **{"w:w": "5000", "w:type": "pct"}))
            for lay in new_pr.findall(dx.qn("w:tblLayout")):
                new_pr.remove(lay)
            t._tbl.replace(tblPr, new_pr)
            _copy_direct_format(ref, t, hdr_rows, (tr or {}).get("body_rows"))
        else:
            st = tblPr.find(dx.qn("w:tblStyle"))
            if st is None:
                tblPr.insert(0, dx.el("w:tblStyle", **{"w:val": "Table"}))
        look = t._tbl.tblPr.find(dx.qn("w:tblLook"))
        if look is None:
            look = dx.el("w:tblLook")
            t._tbl.tblPr.append(look)
        for k, v in (("w:firstRow", "1"), ("w:noHBand", "0"), ("w:lastRow", "0"),
                     ("w:firstColumn", "0"), ("w:lastColumn", "0"), ("w:noVBand", "1")):
            look.set(dx.qn(k), v)
        look.set(dx.qn("w:val"), "04A0")


def _cell_fmt(cell):
    tcPr = cell._tc.tcPr
    shd = copy.deepcopy(tcPr.find(dx.qn("w:shd"))) if tcPr is not None and \
        tcPr.find(dx.qn("w:shd")) is not None else None
    rpr = None
    for p in cell.paragraphs:
        for r in p.runs:
            if r._r.rPr is not None:
                rpr = copy.deepcopy(r._r.rPr)
                break
        if rpr is not None:
            break
    return shd, rpr


def _apply_cell_fmt(cell, fmt):
    shd, rpr = fmt
    if shd is not None:
        tcPr = cell._tc.get_or_add_tcPr()
        for old in tcPr.findall(dx.qn("w:shd")):
            tcPr.remove(old)
        tcPr.append(copy.deepcopy(shd))
    if rpr is not None:
        for p in cell.paragraphs:
            for r in p.runs:
                old = r._r.rPr
                new = copy.deepcopy(rpr)
                if old is not None:
                    # conservar negrita/cursiva/código introducidos en el markdown
                    for tag in ("w:b", "w:i", "w:rStyle"):
                        if old.find(dx.qn(tag)) is not None and new.find(dx.qn(tag)) is None:
                            new.append(copy.deepcopy(old.find(dx.qn(tag))))
                    r._r.remove(old)
                r._r.insert(0, new)


def _copy_direct_format(ref, t, hdr_rows, body_rows=None):
    if not ref.rows:
        return
    hdr_fmt = _cell_fmt(ref.rows[0].cells[-1])
    idx = body_rows if body_rows else range(hdr_rows, len(ref.rows))
    body_fmts = [_cell_fmt(ref.rows[i].cells[-1]) for i in idx]
    for i, row in enumerate(t.rows):
        for cell in row.cells:
            if i < hdr_rows:
                _apply_cell_fmt(cell, hdr_fmt)
            elif body_fmts:
                _apply_cell_fmt(cell, body_fmts[(i - hdr_rows) % len(body_fmts)])


# --------------------------------------------------------------------------- API
def export(src, out=None, toc=True, kind=None, extra_meta=None):
    root = c.project_root()
    cfg = c.load_config(root)
    src = Path(src)
    src = src if src.is_absolute() else (Path.cwd() / src)
    if not src.is_file():
        c.die("no existe %s" % src, c.EXIT_USAGE)
    if not c.which("pandoc"):
        c.die("falta pandoc (brew install pandoc); ver h doctor", c.EXIT_DEP)
    if not dx.HAVE_DOCX:
        c.die("falta python-docx; ver h doctor", c.EXIT_DEP)
    from docx import Document
    is_req = (kind == "requisitos") or src.name == "requisitos.md"
    ecfg = dict(cfg["export"])
    override = (ecfg.get("documents") or {}).get("requisitos" if is_req else (kind or src.stem))
    if override:
        ecfg.update(override)
    template = c.expand(ecfg["template"], root)
    if not template.is_file():
        c.die("no existe la plantilla %s (h template build-base o /plantilla-registrar)" % template)
    mapping = c.load_json(c.expand(ecfg.get("template_map", ""), root), {}) \
        if ecfg.get("template_map") else {}
    dopts = mapping.get("document") or {}
    toc = toc and dopts.get("toc", True)
    text = src.read_text(encoding="utf-8")
    meta = _control_table(text)
    title_m = re.search(r"^#\s+(.+)$", text, flags=re.M)
    default_title, code_suffix = KNOWN_DOCS.get(src.name, (src.stem, "DOC"))
    title = title_m.group(1).strip() if title_m else default_title
    if title_m:
        text = text[:title_m.start()] + text[title_m.end():]
    for sec in dopts.get("strip_sections", []):
        text = re.sub(r"^##\s+%s\s*$.*?(?=^##\s|\Z)" % re.escape(sec), "", text,
                      flags=re.M | re.S)
    if not dopts.get("control_table", True) and meta:
        text = re.sub(r"\A\s*(\|[^\n]*\|\s*\n)+", "\n", text, count=1)
    proj = cfg["project"]
    values = {
        "TITULO": title,
        "SUBTITULO": proj.get("name", ""),
        "PROYECTO": proj.get("name", ""),
        "CODIGO": meta.get("codigo") or "%s-%s" % (proj.get("code", "PRJ"), code_suffix),
        "VERSION": meta.get("version", "0.1"),
        "FECHA": meta.get("fecha") or c.today(),
        "AUTOR": meta.get("autor") or proj.get("author", ""),
        "CLIENTE": proj.get("client", ""),
    }
    values.update(extra_meta or {})
    req_list = rq.parse(src.read_text(encoding="utf-8")) if is_req else []
    rt = mapping.get("requirement_table") or {}
    req_mode = (rt.get("mode") or "per-requirement") if (is_req and rt.get("table_index")
                                                         is not None) else None
    out = Path(out) if out else root / "docs" / "export" / (src.stem + ".docx")
    out.parent.mkdir(parents=True, exist_ok=True)
    cache = root / "build" / "export-cache"
    cache.mkdir(parents=True, exist_ok=True)
    sec_names = {r["seccion"] for r in req_list}
    if req_mode == "rows":
        cfg_secs = {c.norm(x["name"]) for x in cfg["requirements"].get("sections", [])}
        sec_names |= {h for h in re.findall(r"^##\s+(.+?)\s*$", text, flags=re.M)
                      if c.norm(rq.section_name(h)) in cfg_secs}
    pre = Pre(src, cache, _text_width_cm(template) - 0.2, req_mode, sec_names)
    body = pre.run(text)
    with tempfile.TemporaryDirectory() as td:
        md = Path(td) / "doc.md"
        md.write_text(body, encoding="utf-8")
        cmd = ["pandoc", str(md), "-f", "markdown+pipe_tables+implicit_figures+table_captions",
               "-t", "docx",
               "--reference-doc", str(template), "-o", str(out),
               "--resource-path", "%s:%s" % (src.parent, cache),
               "-M", "title=%s" % values["TITULO"], "-M", "subtitle=%s" % values["SUBTITULO"],
               "-M", "author=%s" % values["AUTOR"], "-M", "date=%s" % values["FECHA"],
               "-M", "lang=es-ES", "-M", "toc-title=Índice", "--shift-heading-level-by=-1"]
        if toc:
            cmd += ["--toc", "--toc-depth=3"]
        cp = c.run(cmd, cwd=src.parent)
        if cp.returncode != 0:
            c.die("pandoc falló: %s" % cp.stderr.strip())
    doc = Document(str(out))
    tpl = Document(str(template))
    produced = []
    _apply_cover(doc, tpl, mapping, values)
    if toc:
        _toc_page_break(doc)
    if req_mode:
        _requirements(doc, tpl, mapping, req_list, produced)
    _external_requirements(doc, root, cfg, produced)
    _landscape_sections(doc)
    _style_tables(doc, tpl, mapping, produced)
    _replace_ph_doc(doc, values)
    if not dopts.get("header_footer", True):
        for sp in doc.element.body.iter(dx.qn("w:sectPr")):
            for ref in sp.findall(dx.qn("w:headerReference")) + sp.findall(dx.qn("w:footerReference")):
                sp.remove(ref)
    doc.core_properties.title = values["TITULO"]
    doc.core_properties.author = values["AUTOR"]
    doc.core_properties.subject = values["PROYECTO"]
    doc.save(str(out))
    for w in pre.warnings:
        c.log("WARN", w)
    c.info("exportado: %s" % (out.relative_to(root) if root in out.parents else out))
    return c.EXIT_OK


def export_named(name):
    """Atajos: requisitos, diseno, comparativa, implementacion, oferta, dosier."""
    root = c.project_root()
    paths = {
        "requisitos": "docs/01-requisitos/requisitos.md",
        "diseno": "docs/02-diseno/diseno.md",
        "comparativa": "docs/02-diseno/comparativa.md",
        "implementacion": "docs/03-implementacion/implementacion.md",
        "oferta": "docs/04-oferta/oferta.md",
        "dosier": "docs/dosier/dosier.md",
    }
    if name in paths:
        return export(root / paths[name], kind=name)
    return export(Path(name))

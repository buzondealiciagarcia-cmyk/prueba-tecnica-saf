"""Utilidades de bajo nivel sobre python-docx (XML OOXML)."""
import copy
import re

try:
    from docx import Document  # noqa: F401
    from docx.enum.text import WD_BREAK  # noqa: F401
    from docx.oxml import OxmlElement
    from docx.oxml.ns import qn
    from docx.shared import Pt, RGBColor  # noqa: F401
    HAVE_DOCX = True
except ImportError:  # pragma: no cover - lo informa h doctor
    HAVE_DOCX = False

PH_RE = re.compile(r"\{\{\s*([A-Z_]+)\s*\}\}")


def hex_rgb(h):
    h = h.lstrip("#")
    return RGBColor(int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16))


def el(tag, **attrs):
    e = OxmlElement(tag)
    for k, v in attrs.items():
        e.set(qn(k), str(v))
    return e


def set_cell_shading(cell, fill_hex):
    tcPr = cell._tc.get_or_add_tcPr()
    for old in tcPr.findall(qn("w:shd")):
        tcPr.remove(old)
    tcPr.append(el("w:shd", **{"w:val": "clear", "w:color": "auto", "w:fill": fill_hex.lstrip("#")}))


def page_break_paragraph():
    p = el("w:p")
    r = el("w:r")
    r.append(el("w:br", **{"w:type": "page"}))
    p.append(r)
    return p


def paragraph_has_page_break(p_el):
    for br in p_el.iter(qn("w:br")):
        if br.get(qn("w:type")) == "page":
            return True
    return p_el.find(".//" + qn("w:sectPr")) is not None


def cell_text(cell):
    return "\n".join(p.text for p in cell.paragraphs)


def set_cell_text(cell, text):
    """Sustituye el texto de la celda conservando el formato del primer run."""
    paras = cell.paragraphs
    lines = str(text).split("\n")
    first = paras[0]
    runs = first.runs
    if runs:
        runs[0].text = lines[0]
        for r in runs[1:]:
            r._r.getparent().remove(r._r)
    else:
        first.add_run(lines[0])
    for p in paras[1:]:
        p._p.getparent().remove(p._p)
    for extra in lines[1:]:
        new_p = copy.deepcopy(first._p)
        first._p.addnext(new_p)
        from docx.text.paragraph import Paragraph
        para = Paragraph(new_p, first._parent)
        if para.runs:
            para.runs[0].text = extra
            for r in para.runs[1:]:
                r._r.getparent().remove(r._r)
        first = para


def replace_placeholders_in_element(element_paragraphs, values):
    """Reemplaza {{CLAVE}} en párrafos (une runs del párrafo si es necesario)."""
    for p in element_paragraphs:
        full = "".join(r.text for r in p.runs)
        if "{{" not in full:
            continue
        new = PH_RE.sub(lambda m: str(values.get(m.group(1), m.group(0))), full)
        if new != full and p.runs:
            p.runs[0].text = new
            for r in p.runs[1:]:
                r.text = ""


def iter_paragraphs_deep(container):
    """Párrafos de un documento, celda, cabecera o tabla, incluidas tablas anidadas."""
    if hasattr(container, "rows") and not hasattr(container, "paragraphs"):
        tables = [container]
    else:
        for p in container.paragraphs:
            yield p
        tables = container.tables
    for t in tables:
        for row in t.rows:
            for cell in row.cells:
                for p in iter_paragraphs_deep(cell):
                    yield p


def body_elements(doc):
    return [e for e in doc.element.body if e.tag != qn("w:sectPr")]


def copy_images(src_doc, dst_doc, element):
    """Re-enlaza las imágenes (a:blip r:embed) de un elemento copiado de src a dst."""
    for blip in element.iter(qn("a:blip")):
        rid = blip.get(qn("r:embed"))
        if not rid:
            continue
        part = src_doc.part.related_parts.get(rid)
        if part is None:
            continue
        import io
        new_rid, _ = dst_doc.part.get_or_add_image(io.BytesIO(part.blob))
        blip.set(qn("r:embed"), new_rid)
